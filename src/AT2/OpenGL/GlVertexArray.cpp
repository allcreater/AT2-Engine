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
    if (m_indexBuffer.first)
        m_indexBuffer.first->Bind();
}

GlVertexArray::~GlVertexArray()
{
    glDeleteVertexArrays(1, &m_id);
}

void GlVertexArray::SetIndexBuffer(std::shared_ptr<IVertexBuffer> _buffer, BufferDataType type)
{
    if (!_buffer) //if present we must ensure that it right
    {
        assert(dynamic_cast<GlVertexBuffer*>(_buffer.get()));

        if (_buffer->GetType() != VertexBufferType::IndexBuffer)
            throw AT2Exception("GlVertexBuffer: must be index buffer!");

        if (type != BufferDataType::UByte && type != BufferDataType::UShort && type != BufferDataType::UInt)
            throw AT2Exception("GlVertexBuffer: index buffer must be one of three types: UByte, UShort, UInt");
    }

    const auto* buffer = _buffer.get();
    m_indexBuffer = {std::move(_buffer), buffer ? type : std::optional<BufferDataType>{}};
}

//TODO: one vertex buffer could be binded multiple times with different stride, offsets etc!
void GlVertexArray::SetVertexBuffer(unsigned int _index, std::shared_ptr<IVertexBuffer> _buffer, BufferTypeInfo binding)
{
    if (_buffer)
    {
        assert(dynamic_cast<GlVertexBuffer*>(_buffer.get()));

        if (_buffer->GetType() != VertexBufferType::ArrayBuffer)
            throw AT2Exception("GlVertexBuffer: trying to attach incorrect type buffer");

        const auto platformDataType = Mappings::TranslateExternalType(binding.Type);

        glVertexArrayVertexAttribOffsetEXT(m_id, _buffer->GetId(), _index, static_cast<GLint>(binding.Count),
                                           platformDataType, binding.IsNormalized ? GL_TRUE : GL_FALSE, binding.Stride,
                                           binding.Offset);
        glEnableVertexArrayAttribEXT(m_id, _index);
    }
    else
    {
        glDisableVertexArrayAttribEXT(m_id, _index);
    }

    m_buffers.at(_index) = {std::move(_buffer), binding};
}

void GlVertexArray::SetVertexBufferDivisor(unsigned int index, unsigned int divisor)
{
    glVertexArrayVertexAttribDivisorEXT(m_id, index, divisor);
}

std::shared_ptr<IVertexBuffer> GlVertexArray::GetVertexBuffer(unsigned int index) const
{
    return m_buffers.at(index).first;
}

std::optional<BufferTypeInfo> GlVertexArray::GetVertexBufferBinding(unsigned index) const
{
    const auto& [buffer, binding] = m_buffers.at(index);
    return buffer ? binding : std::optional<BufferTypeInfo> {};
}
