#include "FrameBuffer.h"
#include "Renderer.h"
#include "Mappings.h"
#include "MtlStateManager.h"

using namespace AT2;
using namespace AT2::Metal;


void FrameBuffer::SetAttachmentTexture(MTL::RenderPassAttachmentDescriptor* attachment, MTL::Texture* nativeTexture)
{
    if (nativeTexture)
    {
        attachment->setTexture(nativeTexture);
        attachment->setStoreAction(MTL::StoreActionStore);
    }
    else
    {
        attachment->setStoreAction(MTL::StoreActionDontCare);
    }
    
    //TODO what if size different from attachment to attachment?
    m_size = {nativeTexture->width(), nativeTexture->height()};
}

void FrameBuffer::SetAttachmentTexture(MTL::RenderPassAttachmentDescriptor* attachment, ITexture* texture)
{
    auto* mtlTexture = dynamic_cast<MtlTexture*>(texture);
    if (!mtlTexture && texture)
        throw AT2Exception("texture is not MtlTexture");
    
    SetAttachmentTexture(attachment, mtlTexture->getNativeHandle());
}

void FrameBuffer::SetAttachmentClearColor(MTL::RenderPassColorAttachmentDescriptor* attachment, const std::optional<glm::vec4>& clearColor)
{
    if (clearColor)
    {
        attachment->setClearColor(MTL::ClearColor(clearColor->r, clearColor->g, clearColor->b, clearColor->a));
        attachment->setLoadAction(MTL::LoadActionClear);
    }
    else
        attachment->setLoadAction(MTL::LoadActionDontCare);
}

void FrameBuffer::SetAttachmentClearDepth(MTL::RenderPassDepthAttachmentDescriptor* attachment, const std::optional<float>& clearDepth)
{
    if (clearDepth)
    {
        attachment->setClearDepth(clearDepth.value());
        attachment->setLoadAction(MTL::LoadActionClear);
    }
    else
        attachment->setLoadAction(MTL::LoadActionDontCare);
}


FrameBuffer::FrameBuffer(Renderer& renderer, size_t maxAttachments)
: m_renderer{renderer}
, m_colorAttachments{maxAttachments}
, m_renderPassDescriptor{ConstructMetalObject<MTL::RenderPassDescriptor>()}
{
}

void FrameBuffer::SetColorAttachment(unsigned int attachmentNumber, ColorAttachment attachment) 
{
    if (attachmentNumber >= m_colorAttachments.size())
        throw AT2BufferException( "FrameBuffer: unsupported attachment number");
    
    auto* colorAttachment = m_renderPassDescriptor->colorAttachments()->object(attachmentNumber);
    SetAttachmentClearColor(colorAttachment, attachment.ClearColor);
    SetAttachmentTexture(colorAttachment, attachment.Texture.get());
    
    m_colorAttachments[attachmentNumber] = std::move(attachment);
}

IFrameBuffer::ColorAttachment FrameBuffer::GetColorAttachment(unsigned int attachmentNumber) const
{
    m_colorAttachments.at(attachmentNumber);
}

void FrameBuffer::SetDepthAttachment(DepthAttachment attachment)
{
    auto* depthAttachment = m_renderPassDescriptor->depthAttachment();
    SetAttachmentTexture(depthAttachment, attachment.Texture.get());
    SetAttachmentClearDepth(depthAttachment, attachment.ClearDepth);

    m_depthAttachment = std::move(attachment);
}

IFrameBuffer::DepthAttachment FrameBuffer::GetDepthAttachment() const
{
    return m_depthAttachment;
}

void FrameBuffer::SetClearColor(std::optional<glm::vec4> color) 
{
    for (size_t i = 0; i < m_renderer.GetRendererCapabilities().GetMaxNumberOfColorAttachments(); ++i)
        SetAttachmentClearColor(m_renderPassDescriptor->colorAttachments()->object(i), color);
}

void FrameBuffer::SetClearDepth(std::optional<float> depth) 
{
    SetAttachmentClearDepth(m_renderPassDescriptor->depthAttachment(), depth);
}

namespace {

class PassRenderer : public IRenderer
{
public:
    PassRenderer(Renderer& renderer, MTL::RenderCommandEncoder* encoder) : m_renderer{renderer}, m_renderEncoder{encoder} {}

public:
    void Draw(Primitives::Primitive type, size_t first, long int count, int numInstances, int baseVertex) override
    {
        auto state = Utils::safe_dereference_cast<MtlStateManager&>(&m_renderer.GetStateManager()).GetOrBuildState();
        m_renderEncoder->setRenderPipelineState(state.get());
        
        m_renderEncoder->drawPrimitives(Mappings::TranslatePrimitiveType(type), first, count, numInstances);
    }

    void SetViewport(const AABB2d& viewport) override
    {
        m_renderEncoder->setViewport(MTL::Viewport{viewport.MinBound.x, viewport.MinBound.y, viewport.GetWidth(), viewport.GetHeight(), 0.0f, 1.0f});
    }

    IVisualizationSystem& GetVisualizationSystem() override { return m_renderer; }
    
private:
    Renderer& m_renderer;
    MTL::RenderCommandEncoder* m_renderEncoder;
};

}

void FrameBuffer::Render(RenderFunc renderFunc) 
{
    auto& mtlStateManager = dynamic_cast<MtlStateManager&>(m_renderer.GetStateManager());
    
    auto commandBuffer = Own(m_renderer.getCommandQueue()->commandBuffer());
    auto renderEncoder = Own(commandBuffer->renderCommandEncoder(m_renderPassDescriptor.get()));
 
    PassRenderer renderer{m_renderer, renderEncoder.get()};
    
    mtlStateManager.OnStartRendering(renderEncoder.get());
    renderFunc(renderer);
    mtlStateManager.OnFinishRendering();
    
    renderEncoder->endEncoding();
    OnCommit(commandBuffer.get());
    commandBuffer->commit();
}


// MetalScreenFrameBuffer

void MetalScreenFrameBuffer::Render(RenderFunc renderFunc)
{
    m_drawable = m_swapChain->nextDrawable();
    
    //TODO: is it a crutch or a feature?
    //colorAttachmentDescriptor->setPixelFormat(swapchain->pixelFormat());
    SetAttachmentTexture(m_renderPassDescriptor->colorAttachments()->object(0), m_drawable->texture());
    
    FrameBuffer::Render(std::move(renderFunc));
    
    m_drawable->release();
}

void MetalScreenFrameBuffer::OnCommit(MTL::CommandBuffer* commandBuffer)
{
    commandBuffer->presentDrawable(m_drawable);
}
