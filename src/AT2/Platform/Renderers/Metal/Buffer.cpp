#include "Buffer.h"
#include "Renderer.h"
#include "Mappings.h"

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
        m_buffer = Own(m_renderer.getDevice()->newBuffer(data.data(), data.size(), Mappings::TranslateBufferType(m_type)));
}

void Buffer::ReserveSpace(size_t size)
{
    if (GetLength() >= size)
        return;

    m_buffer = Own(m_renderer.getDevice()->newBuffer(size, Mappings::TranslateBufferType(m_type)));
}

std::span<std::byte> Buffer::Map(BufferOperation usage)
{
    
    //auto sem = dispatch_semaphore_create(1);
    //dispatch_semaphore_wait()
    //dispatch_semaphore_signal(sem);
    
    return MapRange(usage, 0, GetLength());
}

std::span<std::byte> Buffer::MapRange(BufferOperation usage, size_t offset, size_t length)
{
    assert(!m_mappedRange);
    if (!m_buffer)
        throw AT2BufferException("buffer is empty");

    if (offset + length > GetLength())
        throw AT2BufferException("buffer mapping range is out of bounds");

    m_mappedRange = {offset, length};
    return std::span{static_cast<std::byte*>(m_buffer->contents()) + offset, length};
}

void Buffer::Unmap()
{
    assert(m_mappedRange);

    m_buffer->didModifyRange(*m_mappedRange); 
    m_mappedRange.reset();
}
