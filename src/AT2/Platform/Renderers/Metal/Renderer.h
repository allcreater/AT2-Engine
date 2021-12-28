#pragma once

#include "AT2lowlevel.h"

namespace AT2::Metal
{

class Renderer : public IRenderer
{
public:
    NON_COPYABLE_OR_MOVABLE(Renderer)

    explicit Renderer( void* nsWindow );
    ~Renderer() override = default;

public:
    [[nodiscard]] IResourceFactory& GetResourceFactory() const override { return *m_resourceFactory; }
    [[nodiscard]] IStateManager& GetStateManager() const override { return *m_stateManager; }
    [[nodiscard]] IRendererCapabilities& GetRendererCapabilities() const override
    {
        return *m_rendererCapabilities;
    }

    void Shutdown() override;

    void DispatchCompute(glm::uvec3 threadGroupSize) override;
    void Draw(Primitives::Primitive type, size_t first, long int count, int numInstances = 1,
              int baseVertex = 0) override;

    void SetViewport(const AABB2d& viewport) override;
    void ClearBuffer(const glm::vec4& color) override;
    void ClearDepth(float depth) override;
    void FinishFrame() override;

    [[nodiscard]] IFrameBuffer& GetDefaultFramebuffer() const override;

private:
    std::unique_ptr<IStateManager> m_stateManager;
    std::unique_ptr<IResourceFactory> m_resourceFactory;
    std::unique_ptr<IRendererCapabilities> m_rendererCapabilities;
    
    CA::MetalLayer* swapchain;
    MTL::Device* device;
    MTL::Library* library;
    MTL::CommandQueue* commandQueue;
};

}
