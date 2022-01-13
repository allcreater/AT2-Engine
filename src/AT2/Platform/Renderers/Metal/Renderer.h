#pragma once

#include "AT2lowlevel.h"

namespace AT2::Metal
{

class Renderer : public IVisualizationSystem
{
public:
    NON_COPYABLE_OR_MOVABLE(Renderer)

    explicit Renderer( void* nsWindow );
    ~Renderer() override = default;

public:
    [[nodiscard]] IResourceFactory& GetResourceFactory() const override { return *m_resourceFactory; }
    [[nodiscard]] IStateManager& GetStateManager() const override { return *m_stateManager; }
    [[nodiscard]] IRendererCapabilities& GetRendererCapabilities() const override { return *m_rendererCapabilities; }

    void DispatchCompute(glm::uvec3 threadGroupSize) override;

    [[nodiscard]] IFrameBuffer& GetDefaultFramebuffer() const override;

    void BeginFrame() override {}
    void FinishFrame() override {}
    
public: // for internal use only    
    MTL::Device* getDevice() noexcept { return device.get(); }
    MTL::CommandQueue* getCommandQueue() noexcept { return commandQueue.get(); }
    
private:
    std::unique_ptr<IStateManager> m_stateManager;
    std::unique_ptr<IResourceFactory> m_resourceFactory;
    std::unique_ptr<IRendererCapabilities> m_rendererCapabilities;
    std::unique_ptr<IFrameBuffer> m_defaultFramebuffer;
    
    CA::MetalLayer* swapchain;
    MtlPtr<MTL::Device> device;
    MtlPtr<MTL::CommandQueue> commandQueue;
    
};

}
