#include "Renderer.h"

//test
#include "Mappings.h"
#include "ResourceFactory.h"
#include "MtlStateManager.h"

using namespace AT2;
using namespace AT2::Metal;

using namespace std::literals;

// https://github.com/gzorin/sdl-metal-cpp-example

namespace {

template<MTL::GPUFamily ... Family>
bool supportsFamily(MTL::Device* device)
{
    return (device->supportsFamily(Family) || ...) ;
}

class MtlRendererCapabilities : public IRendererCapabilities
{
public:
    MtlRendererCapabilities(Renderer& renderer)
    : m_maxNumberOfColorAttachments{supportsFamily<MTL::GPUFamilyMac1>(renderer.getDevice()) ? 4u : 8u}
    , m_maxTextureSize{supportsFamily<MTL::GPUFamilyMac1, MTL::GPUFamilyMacCatalyst1, MTL::GPUFamilyApple3>(renderer.getDevice()) ? 16384u : 8192u}
    {
    }
    
    [[nodiscard]] unsigned int GetMaxNumberOfTextureUnits() const override { return m_maxNumberOfTextureUnits; }
    [[nodiscard]] unsigned int GetMaxNumberOfColorAttachments() const override { return m_maxNumberOfColorAttachments; }
    [[nodiscard]] unsigned int GetMaxTextureSize() const override { return m_maxTextureSize; }
    [[nodiscard]] unsigned int GetMaxNumberOfVertexAttributes() const override { return m_maxNumberOfVertexAttributes; }

private:
    //TODO
    unsigned int m_maxNumberOfTextureUnits =        31;
    unsigned int m_maxNumberOfColorAttachments;
    unsigned int m_maxTextureSize;
    unsigned int m_maxNumberOfVertexAttributes =    31;
};
}

Renderer::Renderer(void* metalLayer)
{
    swapchain = reinterpret_cast<CA::MetalLayer*>(metalLayer);
    device.reset(swapchain->device());
    
    Log::Info() << "AT2 Metal renderer initialized"sv
                << "Device name: " << device->name()->cString(NS::ASCIIStringEncoding) << '\n';
    
    m_rendererCapabilities = std::make_unique<MtlRendererCapabilities>(*this);
    m_resourceFactory = std::make_unique<ResourceFactory>(*this);
    m_stateManager = std::make_unique<MtlStateManager>(*this);
    
    commandQueue.reset(device->newCommandQueue());
}

void Renderer::DispatchCompute(glm::uvec3 threadGroupSize)
{
    
}

void Renderer::Draw(Primitives::Primitive type, size_t first, long int count, int numInstances, int baseVertex)
{
    assert(frameContext);
    
    if (std::holds_alternative<Primitives::Patches>(type))
        throw AT2Exception("patches rendering is not implemented yet");
        
    //frameContext->renderEncoder->drawPrimitives(Mappings::TranslatePrimitiveType(type), first, count, numInstances);
}

void Renderer::SetViewport(const AABB2d& viewport)
{
    assert(frameContext);
    
    frameContext->renderEncoder->setViewport(MTL::Viewport{viewport.MinBound.x, viewport.MinBound.y, viewport.GetWidth(), viewport.GetHeight(), 0.0f, 1.0f});
}

void Renderer::ClearBuffer(const glm::vec4& color)
{
}

void Renderer::ClearDepth(float depth)
{

}

void Renderer::BeginFrame()
{
    frameContext = FrameContext{};
    frameContext->drawable = swapchain->nextDrawable();
    
    auto defaultPassDescriptor = ConstructMetalObject<MTL::RenderPassDescriptor>();
    auto* colorAttachment = defaultPassDescriptor->colorAttachments()->object(0);
    colorAttachment->setClearColor(MTL::ClearColor(0.0, 0.5, 0.0, 1));
    colorAttachment->setLoadAction(MTL::LoadActionClear);
    colorAttachment->setStoreAction(MTL::StoreActionStore);
    colorAttachment->setTexture(frameContext->drawable->texture());
    
    frameContext->commandBuffer = commandQueue->commandBuffer();
    frameContext->renderEncoder = frameContext->commandBuffer->renderCommandEncoder(defaultPassDescriptor.get());
}

void Renderer::FinishFrame()
{
    frameContext->renderEncoder->endEncoding();
    frameContext->commandBuffer->presentDrawable(frameContext->drawable.get());
    frameContext->commandBuffer->commit();
    
    frameContext.reset();
}


AT2::IFrameBuffer& Renderer::GetDefaultFramebuffer() const
{
    throw AT2::AT2Exception("");
}
