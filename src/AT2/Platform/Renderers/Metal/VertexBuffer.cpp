#include "VertexBuffer.h"

using namespace AT2::Metal;

VertexBuffer::VertexBuffer(VertexBufferType bufferType)
{
	
}

VertexBuffer::~VertexBuffer()
{
	
}

void VertexBuffer::Bind()
{
	
}
     
void VertexBuffer::SetDataRaw(std::span<const std::byte> data)
{
	
}

std::span<std::byte> VertexBuffer::Map(BufferUsage usage)
{
    return std::span<std::byte>();
}

std::span<std::byte> VertexBuffer::MapRange(BufferUsage usage, size_t offset, size_t length)
{
    return std::span<std::byte>();
}

void VertexBuffer::Unmap()
{
	
}
