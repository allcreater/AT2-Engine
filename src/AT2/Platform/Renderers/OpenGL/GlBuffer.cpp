#include "GlBuffer.h"
#include "Mappings.h"
#include "../AT2.h"

using namespace AT2;
using namespace OpenGL;

GlBuffer::GlBuffer(VertexBufferType bufferType)
	: m_id{0}
	, m_publicType{bufferType}
{
    glCreateBuffers(1, &m_id);
    //TODO: use glNamedBufferStorage ?
}


GlBuffer::~GlBuffer()
{
    glDeleteBuffers(1, &m_id);
}

void GlBuffer::SetDataRaw(std::span<const std::byte> data)
{
    assert(!m_mapped);

    glNamedBufferData(m_id, data.size(), data.data(), static_cast<GLenum>(m_usageHint));

    m_length = data.size();
}

std::span<std::byte> GlBuffer::Map(BufferUsage usage)
{
    if (m_mapped)
        throw AT2BufferException("GlBuffer: you must unmap buffer before you could map it again");

    m_mapped = true;

    return {static_cast<std::byte*>(glMapNamedBuffer(m_id, Mappings::TranslateBufferUsage(usage))), m_length};
}

std::span<std::byte> GlBuffer::MapRange(BufferUsage usage, size_t offset, size_t length)
{
    if (m_mapped)
        throw AT2BufferException("GlBuffer: you must unmap buffer before you could map it again");
    if (offset + length > m_length)
        throw std::out_of_range("GlBuffer:MapRange");

    m_mapped = true;

    const GLbitfield mapFlags = GL_MAP_INVALIDATE_RANGE_BIT
        | ((usage | BufferUsage::Read) == BufferUsage::Read ? GL_MAP_READ_BIT : 0)
        | ((usage | BufferUsage::Write) == BufferUsage::Write ? GL_MAP_WRITE_BIT : 0);

    return {static_cast<std::byte*>(glMapNamedBufferRange(m_id, offset, length, mapFlags)), length};
}

void GlBuffer::Unmap()
{
    if (!m_mapped)
        return;

    m_mapped = false;

    glUnmapNamedBuffer(m_id);
}
