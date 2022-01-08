#include "Buffer.h"
#include "Renderer.h"

using namespace AT2::Metal;

Buffer::Buffer(Renderer& renderer, VertexBufferType bufferType)
: renderer{renderer}
, type{bufferType}
{
	
}

Buffer::~Buffer()
{
}

size_t Buffer::GetLength() const noexcept
{
    return buffer ? buffer->length() : 0;
}
     
void Buffer::SetDataRaw(std::span<const std::byte> data)
{
    if (data.empty())
        buffer.reset();
    else
        buffer = renderer.getDevice()->newBuffer(data.data(), data.size(), MTL::ResourceOptionCPUCacheModeDefault);
}

std::span<std::byte> Buffer::Map(BufferUsage usage)
{
    if (!buffer)
        throw AT2BufferException("buffer is empty");

    return {static_cast<std::byte*>(buffer->contents()), buffer->length()};
}

std::span<std::byte> Buffer::MapRange(BufferUsage usage, size_t offset, size_t length)
{
    return Map(usage).subspan(offset).first(length);
}

void Buffer::Unmap()
{
	
}
