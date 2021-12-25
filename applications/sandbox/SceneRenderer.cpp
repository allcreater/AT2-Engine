#include "SceneRenderer.h"
#include "../procedural_meshes.h"
#include "../mesh_renderer.h"

#include <algorithm>
#include <utility>

#include <Scene/Animation.h>

namespace
{

    // Makes temporary array that holds std::transform's computation result
    template <typename T, typename TransformFunc>
    std::span<std::remove_const_t<T>> MakeTransformedSpan(const std::span<T> inputSpan, TransformFunc&& func)
    {
        static thread_local std::vector<std::remove_const_t<T>> temporaryData;
        temporaryData.resize(inputSpan.size());
        std::ranges::transform(inputSpan, temporaryData.begin(), std::forward<TransformFunc>(func));

        return temporaryData;
    }
}

namespace AT2::Scene
{
    RenderVisitor::RenderVisitor(SceneRenderer& renderer, const Camera& camera) :
        camera(camera), scene_renderer(renderer)
    {
    }

    bool RenderVisitor::Visit(Node& node)
    {
        transforms.pushModelView(node.GetTransform());

        auto& stateManager = scene_renderer.renderer->GetStateManager();

        for (auto* meshComponent: node.getComponents<MeshComponent>())
        {
            //TODO: some kind of RenderContext?
            if (active_mesh != meshComponent->getMesh())
            {
                active_mesh = meshComponent->getMesh();

                stateManager.BindShader(active_mesh->Shader);
                stateManager.BindVertexArray(active_mesh->VertexArray);
            }

            if (const auto& skinRef = meshComponent->getSkeletonInstance())
            {
                auto skeletonMatrices = MakeTransformedSpan(
                    skinRef->getResultJointTransforms(),
                    [mvInverse = transforms.getModelViewInverse()](const glm::mat4& transform) { return mvInverse * transform; });

                stateManager.GetActiveShader()->SetUniformArray("u_skeletonMatrices", skeletonMatrices); 
                stateManager.GetActiveShader()->SetUniform("u_useSkinning", 1);
            }
            else
            {
                stateManager.GetActiveShader()->SetUniform("u_useSkinning", 0);
            }


            for (const size_t submeshIndex : meshComponent->GetSubmeshIndices())
            {
                stateManager.GetActiveShader()->SetUniform("u_matModel", transforms.getModelView());
                stateManager.GetActiveShader()->SetUniform(
                    "u_matNormal", glm::mat3(transpose(inverse(camera.getView() * transforms.getModelView()))));

                Utils::MeshRenderer::DrawSubmesh(*scene_renderer.renderer, *active_mesh,
                                                 active_mesh->SubMeshes[submeshIndex]);
            }
        }


        return true;
    }

    void RenderVisitor::UnVisit(Node&) { transforms.popModelView(); }

    LightRenderVisitor::LightRenderVisitor(SceneRenderer& sceneRenderer) : scene_renderer(sceneRenderer) {}

    bool LightRenderVisitor::Visit(Node& node)
    {
        transforms.pushModelView(node.GetTransform());

        if (const auto* lightNode = dynamic_cast<LightNode*>(&node))
        {
            if (!lightNode->GetEnabled())
                return true;

            const auto lightPos = glm::vec3(transforms.getModelView() * glm::vec4 {0, 0, 0, 1});

            if (std::holds_alternative<SphereLight>(lightNode->GetFlavor()))
            {
                collectedLights.push_back({lightPos, lightNode->GetIntensity(), lightNode->GetEffectiveRadius()});
            }
            else if (const auto* skyLight = std::get_if<SkyLight>(&lightNode->GetFlavor()))
            {
                collectedDirectionalLights.push_back({lightPos, lightNode->GetIntensity(),
                                                      glm::mat3(transforms.getModelView()) * skyLight->Direction,
                                                      skyLight->EnvironmentMap});
            }
        }

        return true;
    }

    void LightRenderVisitor::UnVisit(Node&) { transforms.popModelView(); }

    void SceneRenderer::DrawPointLights(const LightRenderVisitor& lrv) const
    {
        using LightAttribs = LightRenderVisitor::LightAttribs;

        //update our vertex buffer...
        auto& rf = renderer->GetResourceFactory();
        auto& vao = lightMesh->VertexArray;


        //TODO: map buffer instead of recreating it
        auto vertexBuffer = rf.MakeVertexBufferFrom(VertexBufferType::ArrayBuffer, lrv.collectedLights);

        vao->SetAttributeBinding(2, vertexBuffer,
            BufferBindingParams {BufferDataType::Float, 3, sizeof(LightAttribs), offsetof(LightAttribs, position), false, 1} );

        vao->SetAttributeBinding(3, vertexBuffer,
            BufferBindingParams {BufferDataType::Float, 3, sizeof(LightAttribs), offsetof(LightAttribs, intensity), false, 1});

        vao->SetAttributeBinding(4, vertexBuffer,
            BufferBindingParams {BufferDataType::Float, 1, sizeof(LightAttribs), offsetof(LightAttribs, effective_radius), false, 1});


        auto& stateManager = renderer->GetStateManager();
        stateManager.BindShader(resources.sphereLightsShader);
        stateManager.BindVertexArray(lightMesh->VertexArray);
        sphereLightsUniforms->Bind(stateManager);

        Utils::MeshRenderer::DrawSubmesh(*renderer, *lightMesh, lightMesh->SubMeshes.front(), lrv.collectedLights.size());
    }

    void SceneRenderer::DrawSkyLight(const LightRenderVisitor& lrv, const Camera& camera) const
    {
        using LightAttribs = LightRenderVisitor::DirectionalLightAttribs;

        const auto comparator = Utils::make_unary_less<LightAttribs>(
            [&](const LightAttribs& lhv) { return length(camera.getPosition() - lhv.position); });

        const auto nearestLightIt =
            std::min_element(lrv.collectedDirectionalLights.begin(), lrv.collectedDirectionalLights.end(), comparator);

        if (nearestLightIt != lrv.collectedDirectionalLights.end())
        {
            skyLightsUniforms->SetUniform("u_lightDirection", nearestLightIt->direction);
            skyLightsUniforms->SetUniform("u_lightIntensity", nearestLightIt->intensity);
            skyLightsUniforms->SetUniform("u_environmentMap", nearestLightIt->environment_map);

            DrawQuad(resources.skyLightsShader, *skyLightsUniforms);
        }
    }

    void SceneRenderer::Initialize(std::shared_ptr<IRenderer> _renderer)
    {
        this->renderer = std::move(_renderer);

        resources.postprocessShader = renderer->GetResourceFactory().CreateShaderProgramFromFiles(
            {"resources/shaders/postprocess.vs.glsl", "resources/shaders/postprocess.fs.glsl"});

        resources.sphereLightsShader = renderer->GetResourceFactory().CreateShaderProgramFromFiles(
            {"resources/shaders/spherelight2.vs.glsl", "resources/shaders/pbr.fs.glsl",
             "resources/shaders/spherelight2.fs.glsl"});

        resources.skyLightsShader = renderer->GetResourceFactory().CreateShaderProgramFromFiles(
            {"resources/shaders/skylight.vs.glsl", "resources/shaders/pbr.fs.glsl",
             "resources/shaders/skylight.fs.glsl"});


        lightMesh = Utils::MakeSphere(*renderer, {32, 16});
        quadMesh = Utils::MakeFullscreenQuadMesh(*renderer);
    }

    void SceneRenderer::ResizeFramebuffers(glm::ivec2 newSize)
    {
        framebuffer_size = newSize;
        dirtyFramebuffers = true;
    }

    //TODO: render context class
    void SceneRenderer::RenderScene(const RenderParameters& params, const ITime& time)
    {
        if (dirtyFramebuffers)
        {
            auto& rf = renderer->GetResourceFactory();

            gBufferFBO = renderer->GetResourceFactory().CreateFrameBuffer();
            gBufferFBO->SetColorAttachment(0, rf.CreateTexture(Texture2D {framebuffer_size}, TextureFormats::RGBA8));   //FragColor
            gBufferFBO->SetColorAttachment(1, rf.CreateTexture(Texture2D {framebuffer_size}, TextureFormats::RGBA32F)); //FragNormal
            gBufferFBO->SetColorAttachment(2, rf.CreateTexture(Texture2D {framebuffer_size}, TextureFormats::RGBA8));   //RoughnessMetallic
            gBufferFBO->SetDepthAttachment(   rf.CreateTexture(Texture2D {framebuffer_size}, TextureFormats::DEPTH32F));

            postProcessFBO = renderer->GetResourceFactory().CreateFrameBuffer();
            postProcessFBO->SetColorAttachment(0, rf.CreateTexture(Texture2D {framebuffer_size}, TextureFormats::RGBA32F));
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

        if (!params.Camera || !params.Scene)
            return;

        SetupCamera(*params.Camera, time);
        renderer->SetViewport(AABB2d {{0, 0}, framebuffer_size});

        auto& stateManager = renderer->GetStateManager();


        // G-buffer pass
        stateManager.BindFramebuffer(gBufferFBO);
        renderer->ClearBuffer({0.0, 0.0, 1.0, 0.0});
        renderer->ClearDepth(1.0);

        stateManager.ApplyState(BlendMode {BlendFactor::SourceAlpha, BlendFactor::OneMinusSourceAlpha});
        stateManager.ApplyState(params.Wireframe ? PolygonRasterizationMode::Lines : PolygonRasterizationMode::Fill);
        stateManager.ApplyState(DepthState {CompareFunction::Less, true, true});
        stateManager.ApplyState(FaceCullMode {false, true});

        RenderVisitor rv {*this, *params.Camera};
        params.Scene->GetRoot().Accept(rv);

        // Lighting pass
        stateManager.BindFramebuffer(postProcessFBO);
        renderer->ClearBuffer({0.0, 0.0, 0.0, 0.0});

        stateManager.ApplyState(PolygonRasterizationMode::Fill);
        stateManager.ApplyState(BlendMode {BlendFactor::SourceAlpha, BlendFactor::One});
        stateManager.ApplyState(DepthState {CompareFunction::Greater, true, false});
        stateManager.ApplyState(FaceCullMode {false, true});

        LightRenderVisitor lrv {*this};
        params.Scene->GetRoot().Accept(lrv);

        DrawPointLights(lrv);

        stateManager.ApplyState(DepthState {CompareFunction::Greater, false, false});
        DrawSkyLight(lrv, *params.Camera);


        // Postprocess pass
        stateManager.BindFramebuffer(params.TargetFramebuffer);
        stateManager.ApplyState(DepthState {CompareFunction::Greater, false, true});
        renderer->ClearBuffer({0.0, 0.0, 0.0, 0.0});
        renderer->ClearDepth(0);

        postprocessUniforms->SetUniform("u_tmExposure", params.Exposure);
        DrawQuad(resources.postprocessShader, *postprocessUniforms);
    }

    void SceneRenderer::SetupCamera(const Camera& camera, const ITime& time)
    {
        if (!cameraUniformBuffer)
        {
            auto buffer = resources.sphereLightsShader->CreateAssociatedUniformStorage("CameraBlock");
            cameraUniformBuffer = std::move(buffer);
        }

        cameraUniformBuffer->SetUniform("u_matView", camera.getView());
        cameraUniformBuffer->SetUniform("u_matInverseView", camera.getViewInverse());
        cameraUniformBuffer->SetUniform("u_matProjection", camera.getProjection());
        cameraUniformBuffer->SetUniform("u_matInverseProjection", camera.getProjectionInverse());
        cameraUniformBuffer->SetUniform("u_matViewProjection", camera.getProjection() * camera.getView());
        cameraUniformBuffer->SetUniform("u_time", time.getTime().count());
        cameraUniformBuffer->Bind(renderer->GetStateManager());
    }

    void SceneRenderer::DrawQuad(const std::shared_ptr<IShaderProgram>& program,
                                 const IUniformContainer& uniformBuffer) const noexcept
    {
        assert(program);

        auto& stateManager = renderer->GetStateManager();
        stateManager.BindShader(program);
        uniformBuffer.Bind(stateManager);
        stateManager.BindVertexArray(quadMesh->VertexArray);
        const auto& primitive = quadMesh->SubMeshes.front().Primitives.front();
        renderer->Draw(primitive.Type, primitive.StartElement, primitive.Count, 1, primitive.BaseVertex);
    }

} // namespace AT2