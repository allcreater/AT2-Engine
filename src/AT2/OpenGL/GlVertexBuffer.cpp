#include "GlVertexBuffer.h"
#include "Mappings.h"

using namespace AT2;

GlVertexBuffer::GlVertexBuffer(VertexBufferType bufferType) : m_id(0), m_publicType(bufferType)
{
    m_privateType = static_cast<GlBufferType>(Mappings::TranslateBufferType(bufferType));

    glGenBuffers(1, &m_id);
}

void GlVertexBuffer::Bind()
{
    glBindBuffer(static_cast<GLenum>(m_privateType), m_id);
}

GlVertexBuffer::~GlVertexBuffer()
{
    glDeleteBuffers(1, &m_id);
}

void GlVertexBuffer::SetData(size_t _size, const void* _data)
{
    assert(__glewNamedBufferDataEXT);
    assert(!m_mapped);

    glNamedBufferDataEXT(m_id, _size, _data, static_cast<GLenum>(m_usageHint));

    m_length = _size;
}

std::byte* GlVertexBuffer::Map(BufferUsage usage)
{
    if (m_mapped)
        throw AT2Exception(AT2Exception::ErrorCase::Buffer,
                               "GlVertexBuffer: you must unmap buffer before you could map it again");

    return static_cast<std::byte*>(glMapNamedBufferEXT(m_id, Mappings::TranslateBufferUsage(usage)));
}

std::byte* GlVertexBuffer::MapRange(BufferUsage usage, size_t offset, size_t length)
{
    if (m_mapped)
        throw AT2Exception(AT2Exception::ErrorCase::Buffer,
                               "GlVertexBuffer: you must unmap buffer before you could map it again");

    const GLbitfield mapFlags = GL_MAP_INVALIDATE_RANGE_BIT
        | ((usage | BufferUsage::Read) == BufferUsage::Read ? GL_MAP_READ_BIT : 0)
        | ((usage | BufferUsage::Write) == BufferUsage::Write ? GL_MAP_WRITE_BIT : 0);

    return static_cast<std::byte*>(glMapNamedBufferRangeEXT(m_id, offset, length, mapFlags));
}

void GlVertexBuffer::Unmap()
{
    if (!m_mapped)
        return;

    glUnmapNamedBufferEXT(m_id);
}
