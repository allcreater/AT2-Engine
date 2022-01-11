#include "FrameBuffer.h"
#include "Renderer.h"

using namespace AT2;
using namespace AT2::Metal;

FrameBuffer::FrameBuffer(const IRendererCapabilities& rendererCapabilities)
{
	
}

FrameBuffer::~FrameBuffer()
{
	
}

void FrameBuffer::SetColorAttachment(unsigned int attachmentNumber, ColorAttachment attachment) 
{
}

const IFrameBuffer::ColorAttachment* FrameBuffer::GetColorAttachment(unsigned int attachmentNumber) const
{
    return nullptr;
}

void FrameBuffer::SetDepthAttachment(DepthAttachment attachment)
{
	
}

const IFrameBuffer::DepthAttachment* FrameBuffer::GetDepthAttachment() const
{
    return nullptr;
}

void FrameBuffer::SetClearColor(std::optional<glm::vec4> color) 
{
}

void FrameBuffer::SetClearDepth(std::optional<float> depth) 
{
}

void FrameBuffer::Render(RenderFunc renderFunc) 
{
}


//

void MetalScreenFrameBuffer::SetClearColor(std::optional<glm::vec4> color)
{
    m_clearColor = color;
}

void MetalScreenFrameBuffer::SetClearDepth(std::optional<float> depth)
{
    m_clearDepth = depth;
}

void MetalScreenFrameBuffer::Render(RenderFunc renderFunc)
{
    std::optional<FrameContext> frameContext = FrameContext{};
    frameContext->drawable = m_swapChain->nextDrawable();
    
    
    auto defaultPassDescriptor = ConstructMetalObject<MTL::RenderPassDescriptor>();
    auto* colorAttachment = defaultPassDescriptor->colorAttachments()->object(0);
    
    if (m_clearColor)
    {
        colorAttachment->setClearColor(MTL::ClearColor(m_clearColor->r, m_clearColor->g, m_clearColor->b, m_clearColor->a));
        colorAttachment->setLoadAction(MTL::LoadActionClear);
    }
    else
        colorAttachment->setLoadAction(MTL::LoadActionDontCare);
        
    if (m_clearDepth)
    {
        defaultPassDescriptor->depthAttachment()->setClearDepth(m_clearDepth.value());
        defaultPassDescriptor->depthAttachment()->setLoadAction(MTL::LoadActionClear);
    }
    else
        defaultPassDescriptor->depthAttachment()->setLoadAction(MTL::LoadActionDontCare);
        
    colorAttachment->setStoreAction(MTL::StoreActionStore);
    colorAttachment->setTexture(frameContext->drawable->texture());
    
    
    frameContext->commandBuffer = m_renderer.getCommandQueue()->commandBuffer();
    frameContext->renderEncoder = frameContext->commandBuffer->renderCommandEncoder(defaultPassDescriptor.get());
    
    //
    renderFunc(m_renderer);
    
    frameContext->renderEncoder->endEncoding();
    frameContext->commandBuffer->presentDrawable(frameContext->drawable.get());
    frameContext->commandBuffer->commit();
}
