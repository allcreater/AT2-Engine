#pragma once

#include <AT2/Scene.h>
#include <AT2/matrix_stack.h>

using namespace AT2;

struct RenderVisitor : NodeVisitor
{
    RenderVisitor(IRenderer& renderer, const Camera& camera);

    void Visit(Node& node) override;

    void UnVisit(Node& node) override;

private:
    const Camera& camera;
    IRenderer& renderer;

    MatrixStack transforms;
    Mesh* active_mesh = nullptr;

};

struct LightRenderVisitor : NodeVisitor
{
    LightRenderVisitor(IRenderer& renderer, const Camera& camera);

    void Visit(Node& node) override;

    void UnVisit(Node& node) override;

    void DrawLights(class SceneRenderer*);

private:
    const Camera& camera;
    IRenderer& renderer;

    MatrixStack transforms;

    

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
    friend struct LightRenderVisitor;

public:
    SceneRenderer() = default;

    void Initialize(std::shared_ptr<IRenderer> renderer);
    void ResizeFramebuffers(glm::ivec2 newSize);
    void RenderScene(Scene& scene, const Camera& camera, IFrameBuffer& targetFramebuffer);


private:
    void SetupCamera(const Camera& camera);
    void DrawQuad(const std::shared_ptr<IShaderProgram>&);

private:
    std::shared_ptr<IRenderer> renderer;
    
    struct Resources
    {
        std::shared_ptr<IShaderProgram> sphereLightsShader, directionalLightsShader, postprocessShader;
    } resources;

    std::unique_ptr<Mesh> lightMesh, quadMesh;
    std::unique_ptr<IUniformContainer> cameraUniformBuffer;
    std::shared_ptr<AT2::IFrameBuffer> gBufferFBO, postProcessFBO;

    bool dirtyFramebuffers = false;
    glm::ivec2 framebuffer_size;
};



std::shared_ptr<MeshNode> MakeTerrain(IRenderer& renderer, std::shared_ptr<IShaderProgram> program, int segX, int segY);
std::unique_ptr<Mesh> MakeSphere(IRenderer& renderer, std::shared_ptr<IShaderProgram> program, int segX, int segY);
std::unique_ptr<Mesh> MakeFullscreenQuadDrawable(const std::shared_ptr<IRenderer>& renderer);