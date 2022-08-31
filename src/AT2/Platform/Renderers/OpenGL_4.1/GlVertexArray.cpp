#include "GlVertexArray.h"
#include <map>
#include "Mappings.h"

using namespace AT2;
using namespace OpenGL41;

GlVertexArray::GlVertexArray(const IRendererCapabilities& _rendererCapabilities) :
    m_buffers(_rendererCapabilities.GetMaxNumberOfVertexAttributes())
{
    glGenVertexArrays(1, &m_id);
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

        if (type != BufferDataType::UByte && type != BufferDataType::UShort && type != BufferDataType::UInt)
            throw AT2Exception("GlBuffer: index buffer must be one of three types: UByte, UShort, UInt");
    }

    m_indexBuffer = {std::move(buffer), buffer ? type : std::optional<BufferDataType> {}};
}


void GlVertexArray::SetAttributeBinding(unsigned int attributeIndex, std::shared_ptr<IBuffer> buffer,
                                    const BufferBindingParams& binding)
{
    glBindVertexArray(m_id);

    if (!buffer)
    {
        glDisableVertexAttribArray(attributeIndex);
        m_buffers.at(attributeIndex).first = nullptr;

        return;
    }

    const auto& glBuffer = Utils::safe_dereference_cast<GlBuffer&>(buffer.get());

    const auto platformDataType = Mappings::TranslateExternalType(binding.Type);
    const auto bindingIndex = attributeIndex;

    glEnableVertexAttribArray(attributeIndex);
    glBindBuffer(GL_ARRAY_BUFFER, glBuffer.GetId());
    glVertexAttribDivisor(bindingIndex, binding.Divisor);

    switch (binding.Type)
    {
        case BufferDataType::Double:
            throw AT2Exception("Not supported");
            break;

        case BufferDataType::Byte:
        case BufferDataType::UByte:
        case BufferDataType::Short:
        case BufferDataType::UShort:
        case BufferDataType::Int:
        case BufferDataType::UInt:
            glVertexAttribIPointer(attributeIndex, static_cast<GLint>(binding.Count), platformDataType, binding.Stride, reinterpret_cast<const void*>(binding.Offset));
            break;

        default:
            glVertexAttribPointer(attributeIndex, static_cast<GLint>(binding.Count), platformDataType, Mappings::TranslateBool(binding.IsNormalized), binding.Stride, reinterpret_cast<const void*>(binding.Offset));
    }

    m_vertexDescriptor.SetVertexAttributeLayout(bindingIndex, VertexAttributeLayout{binding.Type, binding.Count, binding.IsNormalized, bindingIndex, 0});
    if (binding.Divisor > 0)
        m_vertexDescriptor.SetBufferLayout(bindingIndex, BufferBindingParams2{binding.Stride, VertexStepFunc::PerInstance, binding.Divisor});
    else
        m_vertexDescriptor.SetBufferLayout(bindingIndex, BufferBindingParams2{binding.Stride, VertexStepFunc::PerVertex, 1});
    

    m_buffers.at(attributeIndex) = {std::move(buffer), binding};
}

std::shared_ptr<IBuffer> GlVertexArray::GetVertexBuffer(unsigned int index) const
{
    return m_buffers.at(index).first;
}

std::optional<size_t> GlVertexArray::GetLastAttributeIndex() const noexcept
{
    const auto it = std::find_if(m_buffers.crbegin(), m_buffers.crend(), [](const std::pair<std::shared_ptr<IBuffer>, BufferBindingParams>& pair){ return pair.first != nullptr; });
    if (it != m_buffers.crend())
        return m_buffers.size() - std::distance(m_buffers.crbegin(), it) - 1;
    else
        return std::nullopt;
}

std::optional<BufferBindingParams> GlVertexArray::GetVertexBufferBinding(unsigned int index) const
{
    const auto& [buffer, binding] = m_buffers.at(index);
    return buffer ? binding : std::optional<BufferBindingParams> {};
}
