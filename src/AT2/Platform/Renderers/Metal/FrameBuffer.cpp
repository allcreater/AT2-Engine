#include "FrameBuffer.h"

using namespace AT2::Metal;

FrameBuffer::FrameBuffer(const IRendererCapabilities& rendererCapabilities)
{
	
}

FrameBuffer::~FrameBuffer()
{
	
}

void FrameBuffer::Bind()
{
	
}

void FrameBuffer::SetColorAttachment(unsigned int attachmentNumber, const std::shared_ptr<ITexture>& texture)
{
	
}

std::shared_ptr<ITexture> FrameBuffer::GetColorAttachment(unsigned int attachmentNumber) const
{
    return std::shared_ptr<ITexture>();
}

void FrameBuffer::SetDepthAttachment(const std::shared_ptr<ITexture>& texture)
{
	
}

std::shared_ptr<ITexture> FrameBuffer::GetDepthAttachment() const
{
    return std::shared_ptr<ITexture>();
}
