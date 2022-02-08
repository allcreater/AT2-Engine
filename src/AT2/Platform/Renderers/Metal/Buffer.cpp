#include "Buffer.h"
#include "Renderer.h"

using namespace AT2::Metal;

Buffer::Buffer(Renderer& renderer, VertexBufferType bufferType)
: m_renderer{renderer}
, m_type{bufferType}
{
	
}

Buffer::~Buffer()
{
}

size_t Buffer::GetLength() const noexcept
{
    return m_buffer ? m_buffer->length() : 0;
}
     
void Buffer::SetDataRaw(std::span<const std::byte> data)
{
    if (data.empty())
        m_buffer.reset();
    else
        m_buffer = Own(m_renderer.getDevice()->newBuffer(data.data(), data.size(), MTL::ResourceOptionCPUCacheModeDefault));
}

void Buffer::ReserveSpace(size_t size)
{
    if (GetLength() >= size)
        return;

    m_buffer = Own(m_renderer.getDevice()->newBuffer(size, MTL::ResourceOptionCPUCacheModeDefault));
}

std::span<std::byte> Buffer::Map(BufferUsage usage)
{
    if (!m_buffer)
        throw AT2BufferException("buffer is empty");
    //auto sem = dispatch_semaphore_create(1);
    //dispatch_semaphore_wait()
    //dispatch_semaphore_signal(sem);
    
    m_mapped = true;
    return {static_cast<std::byte*>(m_buffer->contents()), m_buffer->length()};
}

std::span<std::byte> Buffer::MapRange(BufferUsage usage, size_t offset, size_t length)
{
    return Map(usage).subspan(offset).first(length);
}

void Buffer::Unmap()
{
    assert(m_mapped);
}
