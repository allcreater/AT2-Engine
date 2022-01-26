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
        buffer = Own(renderer.getDevice()->newBuffer(data.data(), data.size(), MTL::ResourceOptionCPUCacheModeDefault));
}

void Buffer::ReserveSpace(size_t size)
{
    if (GetLength() >= size)
        return;

    constexpr std::byte* emptyData = nullptr;
    SetDataRaw(std::span {emptyData, size});
}

std::span<std::byte> Buffer::Map(BufferUsage usage)
{
    if (!buffer)
        throw AT2BufferException("buffer is empty");
    //auto sem = dispatch_semaphore_create(1);
    //dispatch_semaphore_wait()
    //dispatch_semaphore_signal(sem);
    return {static_cast<std::byte*>(buffer->contents()), buffer->length()};
}

std::span<std::byte> Buffer::MapRange(BufferUsage usage, size_t offset, size_t length)
{
    return Map(usage).subspan(offset).first(length);
}

void Buffer::Unmap()
{
	
}
