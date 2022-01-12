#pragma once

#include <Scene/Scene.h>
#include <matrix_stack.h>

namespace AT2::Scene
{

    class SceneRenderer;

    struct RenderVisitor : NodeVisitor
    {
        RenderVisitor(IRenderer&, SceneRenderer&, const Camera& camera);

        bool Visit(Node& node) override;

        void UnVisit(Node& node) override;

    private:
        IRenderer& renderer;
        const Camera& camera;

        MatrixStack transforms;
        std::shared_ptr<const Mesh> active_mesh;
        SceneRenderer& scene_renderer;
    };


    //TODO: make lights components, collect with usual update visitor
    struct LightRenderVisitor : NodeVisitor
    {
        friend class SceneRenderer;

        LightRenderVisitor(SceneRenderer& sceneRenderer);

        bool Visit(Node& node) override;

        void UnVisit(Node& node) override;

    private:
        SceneRenderer& scene_renderer;

        MatrixStack transforms;

        struct DirectionalLightAttribs
        {
            glm::vec3 position;
            glm::vec3 intensity;
            glm::vec3 direction;
            std::shared_ptr<ITexture> environment_map;
        };
        std::vector<DirectionalLightAttribs> collectedDirectionalLights;

        struct LightAttribs
        {
            glm::vec3 position;
            glm::vec3 intensity;
            float effective_radius;
        };
        std::vector<LightAttribs> collectedLights;
    };

    struct RenderParameters
    {
        Scene* Scene = nullptr;
        const Camera* Camera = nullptr;
        IFrameBuffer* TargetFramebuffer; // not null

        float Exposure = 1.0f;
        bool Wireframe = false;
    };

    class SceneRenderer
    {
        friend struct RenderVisitor;

    public:
        SceneRenderer() = default;

        void Initialize(IVisualizationSystem& renderer);
        void ResizeFramebuffers(glm::ivec2 newSize);
        void RenderScene(IRenderer& renderer, const RenderParameters& params, const ITime& time);

    private:
        void DrawPointLights(IRenderer& renderer, const LightRenderVisitor& lrv) const;
        void DrawSkyLight( IRenderer& renderer, const LightRenderVisitor& lrv, const Camera& camera ) const;

        void SetupCamera(IRenderer& renderer, const Camera& camera, const ITime& time);
        void DrawQuad(IRenderer& renderer, const std::shared_ptr<IShaderProgram>&, const IUniformContainer&) const noexcept;

    private:
        struct Resources
        {
            std::shared_ptr<IShaderProgram> sphereLightsShader, skyLightsShader, postprocessShader;
        } resources;

        std::unique_ptr<Mesh> lightMesh, quadMesh;
        std::unique_ptr<IUniformContainer> cameraUniformBuffer;
        std::shared_ptr<AT2::IFrameBuffer> gBufferFBO, postProcessFBO;

        std::shared_ptr<IUniformContainer> sphereLightsUniforms, skyLightsUniforms, postprocessUniforms;

        glm::ivec2 framebuffer_size = {512, 512};
        bool dirtyFramebuffers = false;
    };

} // namespace AT2