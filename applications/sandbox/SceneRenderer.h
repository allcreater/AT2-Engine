#pragma once

#include <AT2/Scene.h>
#include <AT2/matrix_stack.h>

class SceneRenderer;

using namespace AT2;

struct RenderVisitor : NodeVisitor
{
    RenderVisitor(SceneRenderer&, const Camera& camera);

    void Visit(Node& node) override;

    void UnVisit(Node& node) override;

private:
    const Camera& camera;

    MatrixStack transforms;
    const Mesh* active_mesh = nullptr;
    SceneRenderer& scene_renderer;

};

struct LightRenderVisitor : NodeVisitor
{
    friend class SceneRenderer;

    LightRenderVisitor(SceneRenderer& sceneRenderer);

    void Visit(Node& node) override;

    void UnVisit(Node& node) override;

private:
    SceneRenderer &scene_renderer;

    MatrixStack transforms;

    struct DirectionalLightAttribs
    {
        glm::vec3 intensity;
        glm::vec3 direction;
        std::shared_ptr<ITexture> environment_map;
    };
    std::vector<DirectionalLightAttribs> collectedDirectionalLights;

    struct LightAttribs
    {
        glm::vec4 position;
        glm::vec3 intensity;
        float effective_radius;
    };
    std::vector<LightAttribs> collectedLights;
};

class SceneRenderer
{
    friend struct RenderVisitor;

public:
    SceneRenderer() = default;

    void Initialize(std::shared_ptr<IRenderer> renderer);
    void ResizeFramebuffers(glm::ivec2 newSize);
    void RenderScene(Scene& scene, const Camera& camera, IFrameBuffer& targetFramebuffer, double time);

    void DrawPointLights(const LightRenderVisitor &lrv) const;
    void DrawSkyLight(const LightRenderVisitor& lrv) const;
private:
    void SetupCamera(const Camera& camera);
    void DrawSubmesh(const SubMesh &, int numInstances = 1) const;
    void DrawMesh(const Mesh&, const std::shared_ptr<IShaderProgram>&);
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



std::shared_ptr<MeshNode> MakeTerrain(const IRenderer& renderer, int segX, int segY);
std::unique_ptr<Mesh> MakeSphere(const IRenderer& renderer, int segX, int segY);
std::unique_ptr<Mesh> MakeFullscreenQuadDrawable(const IRenderer& renderer);