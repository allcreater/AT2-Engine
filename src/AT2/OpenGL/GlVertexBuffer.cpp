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
    if (__glewNamedBufferDataEXT)
        glNamedBufferDataEXT(m_id, _size, _data, static_cast<GLenum>(m_usageHint));
    else
        throw AT2Exception(AT2Exception::ErrorCase::Buffer, "GlVertexBuffer: glNamedBufferData not available");

    m_length = _size;
}
