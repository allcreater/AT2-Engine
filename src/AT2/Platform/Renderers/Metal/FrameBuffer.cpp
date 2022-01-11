#include "FrameBuffer.h"

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

const IFrameBuffer::ColorAttachment* GetColorAttachment(unsigned int attachmentNumber) const
{
    return std::shared_ptr<ITexture>();
}

void FrameBuffer::SetDepthAttachment(DepthAttachment attachment)
{
	
}

const IFrameBuffer::DepthAttachment* FrameBuffer::GetDepthAttachment() const
{
    return std::shared_ptr<ITexture>();
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