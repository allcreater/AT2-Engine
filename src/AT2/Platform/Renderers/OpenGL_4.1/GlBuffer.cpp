#include "GlBuffer.h"
#include "Mappings.h"
#include "../AT2.h"

using namespace AT2;
using namespace OpenGL41;

static constexpr GLenum ServiceBindingPoint = GL_ARRAY_BUFFER;

GlBuffer::GlBuffer(VertexBufferType bufferType)
	: m_id{0}
	, m_publicType{bufferType}
{
    glGenBuffers(1, &m_id);
}


GlBuffer::~GlBuffer()
{
    glDeleteBuffers(1, &m_id);
}

void GlBuffer::SetDataRaw(std::span<const std::byte> data)
{
    assert(!m_mapped);

    glBindBuffer(ServiceBindingPoint, m_id);
    glBufferData(ServiceBindingPoint, data.size(), data.data(), Mappings::TranslateBufferUsage(m_publicType)); //glNamedBufferSubdata

    m_length = data.size();
}

void GlBuffer::ReserveSpace( size_t size )
{
    if (m_length >= size)
        return;

    constexpr std::byte* emptyData = nullptr;
    SetDataRaw(std::span {emptyData, size});
}

std::span<std::byte> GlBuffer::Map(BufferOperation usage)
{
    return MapRange(usage, 0, m_length);
}

std::span<std::byte> GlBuffer::MapRange(BufferOperation usage, size_t offset, size_t length)
{
    if (m_mapped)
        throw AT2BufferException("GlBuffer: you must unmap buffer before you could map it again");
    if (offset + length > m_length)
        throw std::out_of_range("GlBuffer:MapRange");

    m_mapped = true;

    const GLbitfield mapFlags = (usage.Contains(BufferOperationFlags::InvalidateRange) ? GL_MAP_INVALIDATE_RANGE_BIT : 0)
        | (usage.Contains(BufferOperationFlags::Read) ? GL_MAP_READ_BIT : 0)
        | (usage.Contains(BufferOperationFlags::Write) ? GL_MAP_WRITE_BIT : 0);

    glBindBuffer(ServiceBindingPoint, m_id);
    return {static_cast<std::byte*>(glMapBufferRange(ServiceBindingPoint, offset, length, mapFlags)), length};
}

void GlBuffer::Unmap()
{
    if (!m_mapped)
        return;

    m_mapped = false;

    glBindBuffer(ServiceBindingPoint, m_id);
    glUnmapBuffer(ServiceBindingPoint);
}
