#pragma once

#include <AT2/Scene.h>
#include <AT2/matrix_stack.h>

namespace AT2::Scene
{

    class SceneRenderer;

    struct RenderVisitor : NodeVisitor
    {
        RenderVisitor(SceneRenderer&, const Camera& camera);

        bool Visit(Node& node) override;

        void UnVisit(Node& node) override;

    private:
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
        Scene& Scene;
        const Camera& Camera;
        IFrameBuffer& TargetFramebuffer;

        double Time = 0.0f;
        bool Wireframe = false;
    };

    class SceneRenderer
    {
        friend struct RenderVisitor;

    public:
        SceneRenderer() = default;

        void Initialize(std::shared_ptr<IRenderer> renderer);
        void ResizeFramebuffers(glm::ivec2 newSize);
        void RenderScene(const RenderParameters& params);

        void DrawPointLights(const LightRenderVisitor& lrv) const;
        void DrawSkyLight(const LightRenderVisitor& lrv, const Camera& camera) const;

    private:
        void SetupCamera(const Camera& camera);
        void DrawQuad(const std::shared_ptr<IShaderProgram>&, const IUniformContainer&) const noexcept;

    private:
        std::shared_ptr<IRenderer> renderer;

        struct Resources
        {
            std::shared_ptr<IShaderProgram> sphereLightsShader, skyLightsShader, postprocessShader;
        } resources;

        std::unique_ptr<Mesh> lightMesh, quadMesh;
        std::unique_ptr<IUniformContainer> cameraUniformBuffer;
        std::shared_ptr<AT2::IFrameBuffer> gBufferFBO, postProcessFBO;

        std::shared_ptr<IUniformContainer> sphereLightsUniforms, skyLightsUniforms, postprocessUniforms;

        bool dirtyFramebuffers = false;
        glm::ivec2 framebuffer_size = {512, 512};
        double time = 0.0;
    };

} // namespace AT2