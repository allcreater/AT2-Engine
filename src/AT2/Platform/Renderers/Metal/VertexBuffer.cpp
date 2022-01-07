#include "VertexBuffer.h"
#include "Renderer.h"

using namespace AT2::Metal;

VertexBuffer::VertexBuffer(Renderer& renderer, VertexBufferType bufferType)
: renderer{renderer}
, type{bufferType}
{
	
}

VertexBuffer::~VertexBuffer()
{
}

size_t VertexBuffer::GetLength() const noexcept
{
    return buffer ? buffer->length() : 0;
}

void VertexBuffer::Bind()
{
	
}
     
void VertexBuffer::SetDataRaw(std::span<const std::byte> data)
{
    if (data.empty())
        buffer.reset();
    else
        buffer = renderer.getDevice()->newBuffer(data.data(), data.size(), MTL::ResourceOptionCPUCacheModeDefault);
}

std::span<std::byte> VertexBuffer::Map(BufferUsage usage)
{
    if (!buffer)
        throw AT2BufferException("buffer is empty");

    return {static_cast<std::byte*>(buffer->contents()), buffer->length()};
}

std::span<std::byte> VertexBuffer::MapRange(BufferUsage usage, size_t offset, size_t length)
{
    return Map(usage).subspan(offset).first(length);
}

void VertexBuffer::Unmap()
{
	
}
