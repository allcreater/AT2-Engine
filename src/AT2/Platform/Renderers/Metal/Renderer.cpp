#include "Renderer.h"

//test
#include "Mappings.h"
#include "ResourceFactory.h"
#include "MtlStateManager.h"
#include "FrameBuffer.h"

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

Renderer::Renderer(IPlatformGraphicsContext& graphicsContext)
{
    swapchain = reinterpret_cast<CA::MetalLayer*>(graphicsContext.getPlatformSwapchain());
    device.reset(swapchain->device());
    
    Log::Info() << "AT2 Metal renderer initialized"sv
                << "Device name: " << device->name()->cString(NS::ASCIIStringEncoding) << '\n';
    
    m_rendererCapabilities = std::make_unique<MtlRendererCapabilities>(*this);
    m_resourceFactory = std::make_unique<ResourceFactory>(*this);
    m_defaultFramebuffer = std::make_unique<MetalScreenFrameBuffer>(*this, swapchain);
    
    commandQueue = Own(device->newCommandQueue());
}

void Renderer::DispatchCompute(const std::shared_ptr<IShaderProgram>& computeProgram, glm::uvec3 threadGroupSize) {
    
}

AT2::IFrameBuffer& Renderer::GetDefaultFramebuffer() const
{
    return *m_defaultFramebuffer;
}
