#include "SceneRenderer.h"
#include "../procedural_meshes.h"
#include "../mesh_renderer.h"

#include <algorithm>
#include <utility>

#include <Scene/Animation.h>

#include "DataLayout/BufferLayout.h"

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
    RenderVisitor::RenderVisitor(IRenderer& renderer, const Camera& camera) :
        renderer {renderer}, camera {camera}
    {
    }

    bool RenderVisitor::Visit(Node& node)
    {
        transforms.pushModelView(node.GetTransform());

        auto& stateManager = renderer.GetStateManager();

        for (auto* meshComponent: node.getComponents<MeshComponent>())
        {
            //TODO: some kind of RenderContext?
            if (active_mesh != meshComponent->getMesh())
            {
                active_mesh = meshComponent->getMesh();

                stateManager.ApplyPipelineState(active_mesh->PipelineState);
                stateManager.BindVertexArray(active_mesh->VertexArray);
            }

            if (const auto& skinRef = meshComponent->getSkeletonInstance())
            {
                auto skeletonMatrices = MakeTransformedSpan(
                    skinRef->getResultJointTransforms(),
                    [mvInverse = transforms.getModelViewInverse()](const glm::mat4& transform) { return mvInverse * transform; });

                stateManager.Commit([&](IUniformsWriter& writer) {
                    writer.Write("u_skeletonMatrices", skeletonMatrices);
                    writer.Write("u_useSkinning", 1);
                });
            }
            else
            {
                stateManager.SetUniform("u_useSkinning", 0);
            }

            for (const size_t submeshIndex : meshComponent->GetSubmeshIndices())
            {
                stateManager.Commit([&](IUniformsWriter& writer) {
                    writer.Write("u_matModel", transforms.getModelView());
                    writer.Write("u_matNormal", glm::mat3(transpose(inverse(camera.getView() * transforms.getModelView()))));
                });

                Utils::MeshRenderer::DrawSubmesh(renderer, *active_mesh, active_mesh->SubMeshes[submeshIndex]);
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

    void SceneRenderer::DrawPointLights(IRenderer& renderer, const LightRenderVisitor& lrv) const
    {
        using LightAttribs = LightRenderVisitor::LightAttribs;

        //update our vertex buffer...
        auto& rf = renderer.GetResourceFactory();
        auto& vao = lightMesh->VertexArray;


        //TODO: map buffer instead of recreating it
        auto vertexBuffer = rf.MakeBufferFrom(VertexBufferFlags::ArrayBuffer, lrv.collectedLights);

        vao->SetAttributeBinding(2, vertexBuffer,
            BufferBindingParams {BufferDataType::Float, 3, sizeof(LightAttribs), offsetof(LightAttribs, position), false, 1} );

        vao->SetAttributeBinding(3, vertexBuffer,
            BufferBindingParams {BufferDataType::Float, 3, sizeof(LightAttribs), offsetof(LightAttribs, intensity), false, 1});

        vao->SetAttributeBinding(4, vertexBuffer,
            BufferBindingParams {BufferDataType::Float, 1, sizeof(LightAttribs), offsetof(LightAttribs, effective_radius), false, 1});


        auto& stateManager = renderer.GetStateManager();
        stateManager.ApplyPipelineState(resources.sphereLightsPipeline);
        stateManager.BindVertexArray(lightMesh->VertexArray);
        sphereLightsUniforms->Bind(stateManager);

        Utils::MeshRenderer::DrawSubmesh(renderer, *lightMesh, lightMesh->SubMeshes.front(), lrv.collectedLights.size());
    }

    void SceneRenderer::DrawSkyLight( IRenderer& renderer, const LightRenderVisitor& lrv, const Camera& camera ) const
    {
        using LightAttribs = LightRenderVisitor::DirectionalLightAttribs;

        const auto comparator = Utils::make_unary_less<LightAttribs>(
            [&](const LightAttribs& lhv) { return length(camera.getPosition() - lhv.position); });

        const auto nearestLightIt =
            std::min_element(lrv.collectedDirectionalLights.begin(), lrv.collectedDirectionalLights.end(), comparator);

        if (nearestLightIt != lrv.collectedDirectionalLights.end())
        {
            skyLightsUniforms->Commit([&](AT2::IUniformsWriter& writer) {
                writer.Write("u_lightDirection", nearestLightIt->direction);
                writer.Write("u_lightIntensity", nearestLightIt->intensity);
                writer.Write("u_environmentMap", nearestLightIt->environment_map);
            });

            renderer.GetStateManager().ApplyPipelineState(resources.skyLightsPipeline);
            DrawQuad(renderer, *skyLightsUniforms);
        }
    }

    void SceneRenderer::Initialize(IVisualizationSystem& visualizationSystem)
    {
        auto& rf = visualizationSystem.GetResourceFactory();

        resources.postprocessShader = rf.CreateShaderProgramFromFiles(
            {"resources/shaders/postprocess.vs.glsl", "resources/shaders/postprocess.fs.glsl"});

        resources.sphereLightsShader = rf.CreateShaderProgramFromFiles(
            {"resources/shaders/spherelight2.vs.glsl", "resources/shaders/pbr.fs.glsl",
             "resources/shaders/spherelight2.fs.glsl"});

        resources.skyLightsShader = rf.CreateShaderProgramFromFiles(
            {"resources/shaders/skylight.vs.glsl", "resources/shaders/pbr.fs.glsl",
             "resources/shaders/skylight.fs.glsl"});

        lightMesh = Utils::MakeSphere(visualizationSystem, {32, 16});
        quadMesh = Utils::MakeFullscreenQuadMesh(visualizationSystem);

        resources.postprocessPipeline = rf.CreatePipelineState(
                    AT2::PipelineStateDescriptor()
                    .SetShader(resources.postprocessShader)
                    .SetVertexArray(quadMesh->VertexArray)
                    .SetBlendMode({})
                    .SetDepthState({}));

        resources.sphereLightsPipeline = rf.CreatePipelineState(
                    AT2::PipelineStateDescriptor()
                    .SetShader(resources.sphereLightsShader)
                    .SetVertexArray(lightMesh->VertexArray)
                    .SetBlendMode({BlendFactor::SourceAlpha, BlendFactor::One})
                    .SetDepthState({CompareFunction::Greater, true, false})); //false, false?

        resources.skyLightsPipeline = rf.CreatePipelineState(
                    AT2::PipelineStateDescriptor()
                    .SetShader(resources.skyLightsShader)
                    .SetVertexArray(quadMesh->VertexArray)
                    .SetBlendMode({BlendFactor::SourceAlpha, BlendFactor::One})
                    .SetDepthState({CompareFunction::Greater, false, false}));
    }

    void SceneRenderer::ResizeFramebuffers(glm::ivec2 newSize)
    {
        framebuffer_size = newSize;
        dirtyFramebuffers = true;
    }

    //TODO: render context class
    void SceneRenderer::RenderScene(IRenderer& renderer, const RenderParameters& params, const ITime& time)
    {
        if (dirtyFramebuffers)
        {
            const auto& rf = renderer.GetResourceFactory();

            gBufferFBO = renderer.GetResourceFactory().CreateFrameBuffer();
            gBufferFBO->SetColorAttachment(0,{ rf.CreateTexture(Texture2D {framebuffer_size}, TextureFormats::RGBA8   ), glm::vec4{0.0, 0.0, 1.0, 0.0}});   //FragColor
            gBufferFBO->SetColorAttachment(1,  rf.CreateTexture(Texture2D {framebuffer_size}, TextureFormats::RGBA32F ) );                                  //FragNormal
            gBufferFBO->SetColorAttachment(2,  rf.CreateTexture(Texture2D {framebuffer_size}, TextureFormats::RGBA8   ) );                                  //RoughnessMetallic
            gBufferFBO->SetDepthAttachment(  { rf.CreateTexture(Texture2D {framebuffer_size}, TextureFormats::DEPTH32F), 1.0f});

            postProcessFBO = renderer.GetResourceFactory().CreateFrameBuffer();
            postProcessFBO->SetColorAttachment(0, {rf.CreateTexture(Texture2D {framebuffer_size}, TextureFormats::RGBA32F), glm::vec4{}});
            postProcessFBO->SetDepthAttachment(gBufferFBO->GetDepthAttachment().Texture); //depth is common with previous stage

            {
                sphereLightsUniforms = std::make_shared<UniformContainer>();

                sphereLightsUniforms->Commit([&](AT2::IUniformsWriter& writer) {
                    writer.Write("u_colorMap", gBufferFBO->GetColorAttachment(0).Texture);
                    writer.Write("u_normalMap", gBufferFBO->GetColorAttachment(1).Texture);
                    writer.Write("u_roughnessMetallicMap", gBufferFBO->GetColorAttachment(2).Texture);
                    writer.Write("u_depthMap", gBufferFBO->GetDepthAttachment().Texture);
                });
            }

            {
                skyLightsUniforms = std::make_shared<UniformContainer>();

                skyLightsUniforms->Commit([&](AT2::IUniformsWriter& writer) {
                    writer.Write("u_colorMap", gBufferFBO->GetColorAttachment(0).Texture);
                    writer.Write("u_normalMap", gBufferFBO->GetColorAttachment(1).Texture);
                    writer.Write("u_roughnessMetallicMap", gBufferFBO->GetColorAttachment(2).Texture);
                    writer.Write("u_depthMap", gBufferFBO->GetDepthAttachment().Texture);
                });
                //skyLightsUniforms->SetUniform("u_environmentMap", 0);
            }

            //Postprocess quad
            {
                postprocessUniforms = std::make_shared<UniformContainer>();
                postprocessUniforms->Commit([&](AT2::IUniformsWriter& writer) {
                    writer.Write("u_colorMap", postProcessFBO->GetColorAttachment(0).Texture);
                    writer.Write("u_depthMap", postProcessFBO->GetDepthAttachment().Texture);
                });
            }

            dirtyFramebuffers = false;
        }

        if (!params.Camera || !params.Scene)
            return;

        if (!cameraUniformBuffer)
            cameraUniformBuffer = resources.sphereLightsShader->CreateAssociatedUniformStorage("CameraBlock");

        VisitAllMeshes(*params.Scene, [&](const MeshRef& mesh) {
            mesh->GetOrCreateDefaultMaterial().SetUniform("CameraBlock", cameraUniformBuffer->GetBuffer());
        });

        SetupCamera(*cameraUniformBuffer, *params.Camera, &time);


        // G-buffer pass
        gBufferFBO->Render([&](IRenderer& renderer) {
			auto& stateManager = renderer.GetStateManager();
            stateManager.ApplyState(params.Wireframe ? PolygonRasterizationMode::Lines : PolygonRasterizationMode::Fill);
            stateManager.ApplyState(FaceCullMode {false, true});

            RenderVisitor rv {renderer, *params.Camera};
            params.Scene->GetRoot().Accept(rv);
        });

        // Lighting pass
        postProcessFBO->Render([&](IRenderer& renderer) {
            auto& stateManager = renderer.GetStateManager();
            stateManager.ApplyState(PolygonRasterizationMode::Fill);
            stateManager.ApplyState(FaceCullMode {false, true});

            LightRenderVisitor lrv {*this};
            params.Scene->GetRoot().Accept(lrv);

            DrawPointLights(renderer, lrv);

            DrawSkyLight(renderer, lrv, *params.Camera );
        });

        // Postprocess pass
        params.TargetFramebuffer->Render([&](IRenderer& renderer) {
            auto& stateManager = renderer.GetStateManager();

            postprocessUniforms->SetUniform("u_tmExposure", params.Exposure);
            stateManager.ApplyPipelineState(resources.postprocessPipeline);
            DrawQuad(renderer, *postprocessUniforms);
        });
    }

    void SceneRenderer::SetupCamera(StructuredBuffer& cameraUniformBuffer, const Camera& camera, const ITime* time)
    {
        cameraUniformBuffer.Commit([&](AT2::IUniformsWriter& writer) {
            writer.Write("u_matView", camera.getView());
            writer.Write("u_matInverseView", camera.getViewInverse());
            writer.Write("u_matProjection", camera.getProjection());
            writer.Write("u_matInverseProjection", camera.getProjectionInverse());
            writer.Write("u_matViewProjection", camera.getProjection() * camera.getView());
            writer.Write("u_time", time ? time->getTime().count() : 0.0 );
        });
    }

    void SceneRenderer::VisitAllMeshes(Scene& scene, const std::function<void(const MeshRef&)>& meshVisitor)
    {
        AT2::Scene::FuncNodeVisitor visitor {[&meshVisitor](AT2::Scene::Node& node) {
            for (auto* meshComponent : node.getComponents<AT2::Scene::MeshComponent>())
                meshVisitor(meshComponent->getMesh());

            return true;
        }};
        scene.GetRoot().Accept(visitor);
    }

    void SceneRenderer::DrawQuad(IRenderer& renderer, const IUniformContainer& uniformBuffer) const noexcept
    {
        auto& stateManager = renderer.GetStateManager();
        uniformBuffer.Bind(stateManager);
        stateManager.BindVertexArray(quadMesh->VertexArray);
        const auto& primitive = quadMesh->SubMeshes.front().Primitives.front();
        renderer.Draw(primitive.Type, primitive.StartElement, primitive.Count, 1, primitive.BaseVertex);
    }

} // namespace AT2
