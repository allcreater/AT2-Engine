#include "GlVertexArray.h"
#include <map>
#include "Mappings.h"

using namespace AT2;
using namespace OpenGL;

GlVertexArray::GlVertexArray(const IRendererCapabilities& _rendererCapabilities) :
    m_buffers(_rendererCapabilities.GetMaxNumberOfVertexAttributes())
{
    glCreateVertexArrays(1, &m_id);
}

GlVertexArray::~GlVertexArray()
{
    glDeleteVertexArrays(1, &m_id);
}

void GlVertexArray::SetIndexBuffer(std::shared_ptr<IBuffer> buffer, BufferDataType type)
{
    if (!buffer) //if present we must ensure that it right
    {
        const auto& glBuffer = Utils::safe_dereference_cast<GlBuffer&>(buffer.get());

        if (glBuffer.GetType() != VertexBufferType::IndexBuffer)
            throw AT2Exception("GlBuffer: must be index buffer!");

        if (type != BufferDataType::UByte && type != BufferDataType::UShort && type != BufferDataType::UInt)
            throw AT2Exception("GlBuffer: index buffer must be one of three types: UByte, UShort, UInt");
    }

    m_indexBuffer = {std::move(buffer), buffer ? type : std::optional<BufferDataType> {}};
}


void GlVertexArray::SetAttributeBinding(unsigned int attributeIndex, std::shared_ptr<IBuffer> buffer,
                                    const BufferBindingParams& binding)
{
    if (!buffer)
    {
        glDisableVertexArrayAttrib(m_id, attributeIndex);
        m_buffers.at(attributeIndex).first = nullptr;

        return;
    }

    const auto& glBuffer = Utils::safe_dereference_cast<GlBuffer&>(buffer.get());

    if (glBuffer.GetType() != VertexBufferType::ArrayBuffer)
        throw AT2Exception("GlBuffer: trying to attach incorrect type buffer");

    const auto platformDataType = Mappings::TranslateExternalType(binding.Type);
    const auto bindingIndex = attributeIndex;


    glVertexArrayVertexBuffer(m_id, bindingIndex, glBuffer.GetId(), binding.Offset, binding.Stride);
    glVertexArrayBindingDivisor(m_id, bindingIndex, binding.Divisor);

    switch (binding.Type)
    {
        case BufferDataType::Double:
            glVertexArrayAttribLFormat(m_id, attributeIndex, static_cast<GLint>(binding.Count), platformDataType, 0);
            break;

        case BufferDataType::Byte:
        case BufferDataType::UByte:
        case BufferDataType::Short:
        case BufferDataType::UShort:
        case BufferDataType::Int:
        case BufferDataType::UInt:
            glVertexArrayAttribIFormat(m_id, attributeIndex, static_cast<GLint>(binding.Count), platformDataType, 0);
            break;

        default:
            glVertexArrayAttribFormat(m_id, attributeIndex, static_cast<GLint>(binding.Count), platformDataType,
                                      Mappings::TranslateBool(binding.IsNormalized), 0);
    }

    glEnableVertexArrayAttrib(m_id, attributeIndex);


    m_buffers.at(attributeIndex) = {std::move(buffer), binding};
}

std::shared_ptr<IBuffer> GlVertexArray::GetVertexBuffer(unsigned int index) const
{
    return m_buffers.at(index).first;
}

std::optional<BufferBindingParams> GlVertexArray::GetVertexBufferBinding(unsigned index) const
{
    const auto& [buffer, binding] = m_buffers.at(index);
    return buffer ? binding : std::optional<BufferBindingParams> {};
}
