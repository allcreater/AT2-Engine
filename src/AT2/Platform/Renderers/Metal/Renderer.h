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

    void DispatchCompute(glm::uvec3 threadGroupSize) override;
    void Draw(Primitives::Primitive type, size_t first, long int count, int numInstances = 1,
              int baseVertex = 0) override;

    void SetViewport(const AABB2d& viewport) override;
    void BeginFrame() override;
    void FinishFrame() override;

    [[nodiscard]] IFrameBuffer& GetDefaultFramebuffer() const override;

public: // for internal use only
    struct FrameContext
    {
        MtlPtr<CA::MetalDrawable> drawable;
        MtlPtr<MTL::CommandBuffer> commandBuffer;
        MtlPtr<MTL::RenderCommandEncoder> renderEncoder;
    };
    
    MTL::Device* getDevice() noexcept { return device.get(); }
    MTL::CommandQueue* getCommandQueue() noexcept { return commandQueue.get(); }
    
    void UpdateStateParams(const std::function<void(MTL::RenderPipelineDescriptor&)>& command)
    {
        if (!m_buildingState)
            m_buildingState = ConstructMetalObject<MTL::RenderPipelineDescriptor>();
        
        command(*m_buildingState);
        m_needNewState = true;
    }
    
private:
    std::unique_ptr<IStateManager> m_stateManager;
    std::unique_ptr<IResourceFactory> m_resourceFactory;
    std::unique_ptr<IRendererCapabilities> m_rendererCapabilities;
    std::unique_ptr<IFrameBuffer> m_defaultFramebuffer;
    
    CA::MetalLayer* swapchain;
    MtlPtr<MTL::Device> device;
    MtlPtr<MTL::CommandQueue> commandQueue;
    
    MtlPtr<MTL::RenderPipelineState> m_activeState;
    MtlPtr<MTL::RenderPipelineDescriptor> m_buildingState;
    bool m_needNewState = true;
    
    std::optional<FrameContext> frameContext;
};

}
