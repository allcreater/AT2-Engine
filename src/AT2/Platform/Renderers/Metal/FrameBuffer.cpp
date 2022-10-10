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

void FrameBuffer::SetColorAttachment(unsigned int attachmentNumber, std::shared_ptr<ITexture> attachment) 
{
    if (attachmentNumber >= m_colorAttachments.size())
        throw AT2BufferException( "FrameBuffer: unsupported attachment number");
    
    auto* colorAttachment = m_renderPassDescriptor->colorAttachments()->object(attachmentNumber);
    SetAttachmentTexture(colorAttachment, attachment.get());
    
    m_colorAttachments[attachmentNumber].Texture = std::move(attachment);
}

IFrameBuffer::ColorAttachment FrameBuffer::GetColorAttachment(unsigned int attachmentNumber) const
{
    return m_colorAttachments.at(attachmentNumber);
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
    for (size_t i = 0; i < m_colorAttachments.size(); ++i)
        SetClearColor(i, color);
}

void FrameBuffer::SetClearColor(unsigned int attachmentNumber, std::optional<glm::vec4> color) 
{
    if (attachmentNumber >= m_colorAttachments.size())
        throw std::out_of_range("attachmentNumber");

    SetAttachmentClearColor(m_renderPassDescriptor->colorAttachments()->object(attachmentNumber), color);
    m_colorAttachments[attachmentNumber].ClearColor = std::move(color);
}

void FrameBuffer::SetClearDepth(std::optional<float> depth) 
{
    SetAttachmentClearDepth(m_renderPassDescriptor->depthAttachment(), depth);
}

void FrameBuffer::Render(RenderFunc renderFunc) 
{
    auto commandBuffer = m_renderer.getCommandQueue()->commandBuffer();
    auto renderEncoder = commandBuffer->renderCommandEncoder(m_renderPassDescriptor.get());
    {
        MtlStateManager renderer{m_renderer, renderEncoder};
        renderFunc(renderer);
    }
    
    renderEncoder->endEncoding();
    OnCommit(commandBuffer);
    commandBuffer->commit();
}


// MetalScreenFrameBuffer

void MetalScreenFrameBuffer::Render(RenderFunc renderFunc)
{
    PrepareAttachments();   
    FrameBuffer::Render(std::move(renderFunc));
}

void MetalScreenFrameBuffer::PrepareAttachments()
{
    m_drawable = m_swapChain->nextDrawable();
    //TODO: is it a crutch or a feature?
    //colorAttachmentDescriptor->setPixelFormat(swapchain->pixelFormat());
    //SetAttachmentTexture(m_renderPassDescriptor->colorAttachments()->object(0), m_drawable->texture());
    
    {
        auto texture = std::make_shared<MtlTexture>(GetVisualisationSystem(), m_drawable->texture());
        SetColorAttachment(0, std::move(texture));
    }

    if (auto depthTexture = GetDepthAttachment().Texture; !depthTexture || glm::xy(depthTexture->GetSize()) != GetActualSize())
    {
        auto newTexture = GetVisualisationSystem().GetResourceFactory().CreateTexture(Texture2D{TextureFormat::Depth32Float, GetActualSize()}, true);
        SetDepthAttachment({std::move(newTexture), 1.0f});
    }
}

void MetalScreenFrameBuffer::OnCommit(MTL::CommandBuffer* commandBuffer)
{
    commandBuffer->presentDrawable(m_drawable);
    m_drawable = nullptr;
}
