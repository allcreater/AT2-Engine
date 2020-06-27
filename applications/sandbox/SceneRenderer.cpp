#include "SceneRenderer.h"

#include <utility>

#include "AT2/OpenGL/GlFrameBuffer.h"
#include "AT2/OpenGL/GlShaderProgram.h"
#include "AT2/OpenGL/GlUniformBuffer.h"


RenderVisitor::RenderVisitor(SceneRenderer& renderer, const Camera& camera):
    camera(camera),
    scene_renderer(renderer)
{
}

void RenderVisitor::Visit(Node& node)
{
    transforms.pushModelView(node.GetTransform());

    auto &stateManager = scene_renderer.renderer->GetStateManager();

    if (const auto* meshNode = dynamic_cast<MeshNode*>(&node))
    {
        active_mesh = &meshNode->GetMesh(); //TODO: some kind of RenderContext?

        stateManager.BindShader(active_mesh->Shader);
        stateManager.BindVertexArray(active_mesh->VertexArray);

        if (active_mesh->UniformBuffer)
            active_mesh->UniformBuffer->Bind(stateManager);
    }
    else if (const auto* submeshNode = dynamic_cast<DrawableNode*>(&node))
    {
        if (!active_mesh)
            return;

        stateManager.GetActiveShader()->SetUniform("u_matModel", transforms.getModelView());
        stateManager.GetActiveShader()->SetUniform("u_matNormal", glm::mat3(transpose(inverse(camera.getView() * transforms.getModelView()))));

        const auto& submesh = active_mesh->Submeshes[submeshNode->SubmeshIndex];
        scene_renderer.DrawSubmesh(submesh);
    }

}

void RenderVisitor::UnVisit(Node& node)
{
    transforms.popModelView();
}

LightRenderVisitor::LightRenderVisitor(SceneRenderer& sceneRenderer) :
    scene_renderer(sceneRenderer)
{
}

void LightRenderVisitor::Visit(Node& node)
{
    transforms.pushModelView(node.GetTransform());

    if (const auto* lightNode = dynamic_cast<LightNode*>(&node))
    {
        if (std::holds_alternative<SphereLight>(lightNode->GetFlavor()))
        {
            collectedLights.push_back({
                transforms.getModelView() * glm::vec4{0,0,0,1},
                lightNode->GetIntensity(),
                lightNode->GetEffectiveRadius()
            });
        }
        else if (const auto* skyLight = std::get_if<SkyLight>(&lightNode->GetFlavor()))
        {
            collectedDirectionalLights.push_back({
                lightNode->GetIntensity(),
                glm::mat3(transforms.getModelView()) * skyLight->Direction,
                skyLight->EnvironmentMap
            });
        }
    }

}

void LightRenderVisitor::UnVisit(Node& node)
{
    transforms.popModelView();
}

void SceneRenderer::DrawPointLights(const LightRenderVisitor &lrv) const
{
    //update our vertex buffer...
    auto &rf = renderer->GetResourceFactory();
    auto &vao = lightMesh->VertexArray;


    using LightAttribs = LightRenderVisitor::LightAttribs;
    vao->SetVertexBuffer(2,
        rf.CreateVertexBuffer(VertexBufferType::ArrayBuffer,
            BufferTypeInfo{ BufferDataType::Float, 4, sizeof(LightAttribs), offsetof(LightAttribs, position) },
            lrv.collectedLights.size() * sizeof(LightAttribs),
            lrv.collectedLights.data())
    );
    vao->SetVertexBufferDivisor(2, 1);

    vao->SetVertexBuffer(3,
        rf.CreateVertexBuffer(VertexBufferType::ArrayBuffer,
            BufferTypeInfo{ BufferDataType::Float, 3, sizeof(LightAttribs), offsetof(LightAttribs, intensity) },
            lrv.collectedLights.size() * sizeof(LightAttribs),
            lrv.collectedLights.data())
    );
    vao->SetVertexBufferDivisor(3, 1);

    vao->SetVertexBuffer(4,
        rf.CreateVertexBuffer(VertexBufferType::ArrayBuffer,
            BufferTypeInfo{ BufferDataType::Float, 1, sizeof(LightAttribs), offsetof(LightAttribs, effective_radius) },
            lrv.collectedLights.size() * sizeof(LightAttribs),
            lrv.collectedLights.data())
    );
    vao->SetVertexBufferDivisor(4, 1);


    auto &stateManager = renderer->GetStateManager();
    stateManager.BindShader(resources.sphereLightsShader);
    stateManager.BindVertexArray(lightMesh->VertexArray);
    sphereLightsUniforms->Bind(stateManager);

    DrawSubmesh(lightMesh->Submeshes.front(), lrv.collectedLights.size());

}

void SceneRenderer::DrawSkyLight(const LightRenderVisitor& lrv) const
{
    auto& stateManager = renderer->GetStateManager();
    //usually it's number significant less than spherical, so that we will draw them as usually
    for (const auto& directionalLight : lrv.collectedDirectionalLights)
    {
        skyLightsUniforms->SetUniform("u_lightDirection", directionalLight.direction);
        skyLightsUniforms->SetUniform("u_lightIntensity", directionalLight.intensity);
        skyLightsUniforms->SetUniform("u_environmentMap", directionalLight.environment_map);

        DrawQuad(resources.skyLightsShader, *skyLightsUniforms);
    }
}

void SceneRenderer::Initialize(std::shared_ptr<IRenderer> renderer)
{

    resources.postprocessShader = renderer->GetResourceFactory().CreateShaderProgramFromFiles({
    "resources/shaders/postprocess.vs.glsl",
    "resources/shaders/postprocess.fs.glsl" });

    resources.sphereLightsShader = renderer->GetResourceFactory().CreateShaderProgramFromFiles({
    "resources/shaders/spherelight2.vs.glsl",
    "resources/shaders/pbr.fs.glsl",
    "resources/shaders/spherelight2.fs.glsl" });

    resources.skyLightsShader = renderer->GetResourceFactory().CreateShaderProgramFromFiles({
        "resources/shaders/skylight.vs.glsl",
        "resources/shaders/pbr.fs.glsl",
        "resources/shaders/skylight.fs.glsl" });


    lightMesh = MakeSphere(*renderer, 32, 16);
    quadMesh = MakeFullscreenQuadDrawable(*renderer);

    this->renderer = std::move(renderer);
}

void SceneRenderer::ResizeFramebuffers(glm::ivec2 newSize)
{
    framebuffer_size = newSize;
    dirtyFramebuffers = true;
}

//TODO: render context class
void SceneRenderer::RenderScene(Scene& scene, const Camera& camera, IFrameBuffer& targetFramebuffer, double time)
{
    this->time = time;

    if (dirtyFramebuffers)
    {
        auto& rf = renderer->GetResourceFactory();

        gBufferFBO = std::make_shared<AT2::GlFrameBuffer>(renderer->GetRendererCapabilities());
        gBufferFBO->SetColorAttachment(0, rf.CreateTexture(Texture2D{ framebuffer_size }, TextureFormats::RGBA8));
        gBufferFBO->SetColorAttachment(1, rf.CreateTexture(Texture2D{ framebuffer_size }, TextureFormats::RGBA32F));
        gBufferFBO->SetColorAttachment(2, rf.CreateTexture(Texture2D{ framebuffer_size }, TextureFormats::RGBA8));
        gBufferFBO->SetDepthAttachment(rf.CreateTexture(Texture2D{ framebuffer_size }, TextureFormats::DEPTH32F));

        postProcessFBO = std::make_shared<AT2::GlFrameBuffer>(renderer->GetRendererCapabilities());
        postProcessFBO->SetColorAttachment(0, rf.CreateTexture(Texture2D{ framebuffer_size }, TextureFormats::RGBA32F));
        postProcessFBO->SetDepthAttachment(gBufferFBO->GetDepthAttachment()); //depth is common with previous stage

        {
            sphereLightsUniforms = resources.sphereLightsShader->CreateAssociatedUniformStorage();
            //sphereLightsUniforms->SetUniform("u_texNoise", Noise3Tex);
            sphereLightsUniforms->SetUniform("u_colorMap", gBufferFBO->GetColorAttachment(0));
            sphereLightsUniforms->SetUniform("u_normalMap", gBufferFBO->GetColorAttachment(1));
            sphereLightsUniforms->SetUniform("u_roughnessMetallicMap", gBufferFBO->GetColorAttachment(2));
            sphereLightsUniforms->SetUniform("u_depthMap", gBufferFBO->GetDepthAttachment());

        }

        {
            skyLightsUniforms = resources.skyLightsShader->CreateAssociatedUniformStorage();
            //skyLightsUniforms->SetUniform("u_texNoise", Noise3Tex);
            skyLightsUniforms->SetUniform("u_colorMap", gBufferFBO->GetColorAttachment(0));
            skyLightsUniforms->SetUniform("u_normalMap", gBufferFBO->GetColorAttachment(1));
            skyLightsUniforms->SetUniform("u_roughnessMetallicMap", gBufferFBO->GetColorAttachment(2));
            skyLightsUniforms->SetUniform("u_depthMap", gBufferFBO->GetDepthAttachment());
            //skyLightsUniforms->SetUniform("u_environmentMap", 0);
        }

        //Postprocess quad
        {
            postprocessUniforms = resources.postprocessShader->CreateAssociatedUniformStorage();
            //uniformStorage->SetUniform("u_texNoise", Noise3Tex);
            postprocessUniforms->SetUniform("u_colorMap", postProcessFBO->GetColorAttachment(0));
            postprocessUniforms->SetUniform("u_depthMap", postProcessFBO->GetDepthAttachment());
        }
        
        dirtyFramebuffers = false;
    }

    SetupCamera(camera);

    renderer->SetViewport(AABB2d{ {0, 0}, framebuffer_size });

    gBufferFBO->Bind();
    renderer->ClearBuffer(glm::vec4(0.0, 0.0, 1.0, 0.0));
    renderer->ClearDepth(1.0);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glPolygonMode(GL_FRONT_AND_BACK, (WireframeMode) ? GL_LINE : GL_FILL);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glCullFace(GL_FRONT);


    //objects
    RenderVisitor rv { *this, camera };
    scene.GetRoot().Accept(rv);


    postProcessFBO->Bind();

    renderer->ClearBuffer(glm::vec4(0.0, 0.0, 0.0, 0.0));
    //glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_GREATER);
    glCullFace(GL_BACK);

    //lights
    LightRenderVisitor lrv{ *this };
    scene.GetRoot().Accept(lrv);

    DrawPointLights(lrv);
    glDisable(GL_DEPTH_TEST);
    DrawSkyLight(lrv);


    targetFramebuffer.Bind();
    glDepthMask(GL_TRUE);
    renderer->ClearBuffer(glm::vec4(0.0, 0.0, 0.0, 0.0));
    renderer->ClearDepth(0);

    glCullFace(GL_BACK);
    glDisable(GL_DEPTH_TEST);


    DrawQuad(resources.postprocessShader, *postprocessUniforms);
}

void SceneRenderer::SetupCamera(const Camera& camera)
{
    if (!cameraUniformBuffer)
    {
        auto buffer =  std::make_unique<AT2::GlUniformBuffer>(std::dynamic_pointer_cast<AT2::GlShaderProgram>(resources.sphereLightsShader)->GetUniformBlockInfo("CameraBlock"));
        buffer->SetBindingPoint(1);
        cameraUniformBuffer = std::move(buffer);
    }

    cameraUniformBuffer->SetUniform("u_matView", camera.getView());
    cameraUniformBuffer->SetUniform("u_matInverseView", camera.getViewInverse());
    cameraUniformBuffer->SetUniform("u_matProjection", camera.getProjection());
    cameraUniformBuffer->SetUniform("u_matInverseProjection", camera.getProjectionInverse());
    cameraUniformBuffer->SetUniform("u_matViewProjection", camera.getProjection() * camera.getView());
    cameraUniformBuffer->SetUniform("u_time", time);
    cameraUniformBuffer->Bind(renderer->GetStateManager());

}

void SceneRenderer::DrawSubmesh(const SubMesh & subMesh, int numInstances) const
{
    auto &stateManager = renderer->GetStateManager();
    //stateManager.BindTextures(subMesh.Textures);
    if (subMesh.UniformBuffer)
        subMesh.UniformBuffer->Bind(stateManager);

    for (const auto &primitive : subMesh.Primitives)
        renderer->Draw(primitive.Type, primitive.StartElement, primitive.Count, numInstances, primitive.BaseVertex);
}

void SceneRenderer::DrawMesh(const Mesh& mesh, const std::shared_ptr<IShaderProgram>& program)
{
    auto& stateManager = renderer->GetStateManager();

    if (program)
        stateManager.BindShader(program);

    stateManager.BindVertexArray(mesh.VertexArray);

    for (const auto &submesh : mesh.Submeshes)
        DrawSubmesh(submesh);
}

void SceneRenderer::DrawQuad(const std::shared_ptr<IShaderProgram>& program, const IUniformContainer& uniformBuffer) const noexcept
{
    assert(program);

    auto& stateManager = renderer->GetStateManager();
    stateManager.BindShader(program);
    uniformBuffer.Bind(stateManager);
    stateManager.BindVertexArray(quadMesh->VertexArray);
    const auto primitive = quadMesh->Submeshes.front().Primitives.front();
    renderer->Draw(primitive.Type, primitive.StartElement, primitive.Count, 1, primitive.BaseVertex);
}



std::shared_ptr<MeshNode> MakeTerrain(const IRenderer& renderer, int segX, int segY)
{
    assert(segX < 1024 && segY < 1024);

    std::vector<glm::vec2> texCoords(segX * segY * 4);//TODO! GlVertexBuffer - take iterators!

    for (int j = 0; j < segY; ++j)
    for (int i = 0; i < segX; ++i)
    {
        const auto num = (i + j * segX) * 4;
        texCoords[num] = glm::vec2(float(i) / segX, float(j) / segY);
        texCoords[num + 1] = glm::vec2(float(i + 1) / segX, float(j) / segY);
        texCoords[num + 2] = glm::vec2(float(i + 1) / segX, float(j + 1) / segY);
        texCoords[num + 3] = glm::vec2(float(i) / segX, float(j + 1) / segY);
    }


    auto& rf = renderer.GetResourceFactory();
    auto vao = rf.CreateVertexArray();
    vao->SetVertexBuffer(1, rf.CreateVertexBuffer(VertexBufferType::ArrayBuffer, AT2::BufferDataTypes::Vec2, texCoords.size() * sizeof(glm::vec2), texCoords.data()));

    auto rootNode = std::make_shared<MeshNode>();

    Mesh& mesh = rootNode->GetMesh();
    mesh.Name = "Terrain";
    mesh.VertexArray = vao;

    SubMesh subMesh;
    subMesh.Primitives.emplace_back(Primitives::Patches{4}, 0, texCoords.size());

    mesh.Submeshes.push_back(std::move(subMesh));


    auto drawable = std::make_shared<DrawableNode>();
    drawable->SetName("Terrain drawable");
    drawable->SubmeshIndex = 0;

    rootNode->AddChild(std::move(drawable));

    return rootNode;
}

std::unique_ptr<Mesh> MakeSphere(const IRenderer& renderer,  int segX, int segY)
{
    assert(segX <= 1024 && segY <= 512);

    std::vector<glm::vec3> normals; normals.reserve(segX * segY);
    std::vector<glm::uint> indices; indices.reserve(segX * segY * 6);

    for (int j = 0; j < segY; ++j)
    {
        const double angV = j * pi / (segY - 1);
        for (int i = 0; i < segX; ++i)
        {
            const double angH = i * pi * 2 / segX;

            normals.emplace_back(sin(angV) * cos(angH), sin(angV) * sin(angH), cos(angV));
        }
    }

    for (int j = 0; j < segY - 1; ++j)
    {
        const int nj = j + 1;
        for (int i = 0; i < segX; ++i)
        {
            const int ni = (i + 1) % segX;

            indices.push_back(j * segX + i);
            indices.push_back(j * segX + ni);
            indices.push_back(nj * segX + ni);
            indices.push_back(j * segX + i);
            indices.push_back(nj * segX + ni);
            indices.push_back(nj * segX + i);
        }
    }

    auto& rf = renderer.GetResourceFactory();

    auto mesh = std::make_unique<Mesh>();

    mesh->VertexArray = rf.CreateVertexArray();
    mesh->VertexArray->SetVertexBuffer(1, rf.CreateVertexBuffer(VertexBufferType::ArrayBuffer, AT2::BufferDataTypes::Vec3, normals.size() * sizeof(glm::vec3), normals.data()));
    mesh->VertexArray->SetIndexBuffer(rf.CreateVertexBuffer(VertexBufferType::IndexBuffer, AT2::BufferDataTypes::UInt, indices.size() * sizeof(glm::uint), indices.data()));


    //don't know how to make it better
    SubMesh subMesh;
    subMesh.Primitives.emplace_back(Primitives::Triangles{}, 0, indices.size());
    mesh->Submeshes.push_back(std::move(subMesh));

    return mesh;
}

std::unique_ptr<Mesh> MakeFullscreenQuadDrawable(const IRenderer& renderer)
{
    static glm::vec3 positions[] = { glm::vec3(-1.0, -1.0, -1.0), glm::vec3(1.0, -1.0, -1.0), glm::vec3(1.0, 1.0, -1.0), glm::vec3(-1.0, 1.0, -1.0) };

    auto& rf = renderer.GetResourceFactory();
    auto vao = rf.CreateVertexArray();
    vao->SetVertexBuffer(1, rf.CreateVertexBuffer(VertexBufferType::ArrayBuffer, AT2::BufferDataTypes::Vec3, 4 * sizeof(glm::vec3), positions));


    SubMesh subMesh;
    subMesh.Primitives.emplace_back(Primitives::TriangleFan{}, 0, 4);

    auto mesh = std::make_unique<Mesh>();
    mesh->VertexArray = vao;
    mesh->Submeshes.push_back(std::move(subMesh));

    return mesh;
}