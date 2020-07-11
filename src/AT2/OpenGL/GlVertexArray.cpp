#include "GlVertexArray.h"
#include <map>
#include "Mappings.h"

using namespace AT2;

GlVertexArray::GlVertexArray(const IRendererCapabilities& _rendererCapabilities) :
    m_buffers(_rendererCapabilities.GetMaxNumberOfVertexAttributes())
{
    glGenVertexArrays(1, &m_id);
}

//TODO: unlink vertex array from index buffer?
void GlVertexArray::Bind()
{
    glBindVertexArray(m_id);
    if (m_indexBuffer)
        m_indexBuffer->Bind();
}

GlVertexArray::~GlVertexArray()
{
    glDeleteVertexArrays(1, &m_id);
}

void GlVertexArray::SetIndexBuffer(std::shared_ptr<IVertexBuffer> _buffer)
{
    if (_buffer) //if present we must ensure that it right
    {
        assert(dynamic_cast<GlVertexBuffer*>(_buffer.get()));

        if (_buffer->GetType() != VertexBufferType::IndexBuffer)
            throw AT2Exception("GlVertexBuffer: must be index buffer!");

        if (_buffer->GetDataType().Type != BufferDataType::UByte &&
            _buffer->GetDataType().Type != BufferDataType::UShort &&
            _buffer->GetDataType().Type != BufferDataType::UInt)
            throw AT2Exception("GlVertexBuffer: index buffer must be one of three types: UByte, UShort, UInt");
    }

    m_indexBuffer = std::move(_buffer);
}

std::shared_ptr<IVertexBuffer> GlVertexArray::GetIndexBuffer() const
{
    return m_indexBuffer;
}
//TODO: one vertex buffer could be binded multiple times with different stride, offsets etc!
void GlVertexArray::SetVertexBuffer(unsigned int _index, std::shared_ptr<IVertexBuffer> _buffer)
{
    if (_buffer)
    {
        assert(dynamic_cast<GlVertexBuffer*>(_buffer.get()));

        if (_buffer->GetType() != VertexBufferType::ArrayBuffer)
            throw AT2Exception("GlVertexBuffer: trying to attach incorrect type buffer");

        const auto platformDataType = Mappings::TranslateExternalType(_buffer->GetDataType().Type);

        glVertexArrayVertexAttribOffsetEXT(m_id, _buffer->GetId(), _index,
                                           static_cast<GLint>(_buffer->GetDataType().Count), platformDataType,
                                           _buffer->GetDataType().IsNormalized ? GL_TRUE : GL_FALSE,
                                           _buffer->GetDataType().Stride, _buffer->GetDataType().Offset);
        glEnableVertexArrayAttribEXT(m_id, _index);
    }
    else
    {
        glDisableVertexArrayAttribEXT(m_id, _index);
    }

    m_buffers.at(_index) = std::move(_buffer);
}

void GlVertexArray::SetVertexBufferDivisor(unsigned int index, unsigned int divisor)
{
    glVertexArrayVertexAttribDivisorEXT(m_id, index, divisor);
}

std::shared_ptr<IVertexBuffer> GlVertexArray::GetVertexBuffer(unsigned int _index) const
{
    return m_buffers.at(_index);
}
