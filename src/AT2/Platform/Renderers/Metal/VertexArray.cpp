#include "VertexArray.h"
#include "Mappings.h"

using namespace AT2;
using namespace Metal;

VertexArray::VertexArray(const IRendererCapabilities& rendererCapabilities)
: m_buffers(rendererCapabilities.GetMaxNumberOfVertexAttributes())
{
}

VertexArray::~VertexArray()
{
}

void VertexArray::SetIndexBuffer(std::shared_ptr<IBuffer> buffer, BufferDataType type)
{
    m_indexBuffer = {std::move(buffer), buffer ? type : std::optional<BufferDataType> {}};
}

void VertexArray::SetAttributeBinding(unsigned int attributeIndex, std::shared_ptr<IBuffer> buffer,
                                      const BufferBindingParams& binding)
{
    const auto bindingIndex = attributeIndex;

    m_vertexDescriptor.SetVertexAttributeLayout(bindingIndex, VertexAttributeLayout{binding.Type, binding.Count, binding.IsNormalized, bindingIndex, 0});
    if (binding.Divisor > 0)
        m_vertexDescriptor.SetBufferLayout(bindingIndex, BufferBindingParams2{binding.Stride, VertexStepFunc::PerInstance, binding.Divisor});
    else
        m_vertexDescriptor.SetBufferLayout(bindingIndex, BufferBindingParams2{binding.Stride, VertexStepFunc::PerVertex, 1});
    
    
    m_buffers.at(attributeIndex) = { buffer, binding };
}

std::shared_ptr<IBuffer> VertexArray::GetVertexBuffer(unsigned int index) const
{
    return m_buffers.at(index).first;
}

std::optional<size_t> VertexArray::GetLastAttributeIndex() const noexcept
{
    const auto it = std::find_if(m_buffers.crbegin(), m_buffers.crend(), [](const std::pair<std::shared_ptr<IBuffer>, BufferBindingParams>& pair){ return pair.first != nullptr; });
    if (it != m_buffers.crend())
        return m_buffers.size() - std::distance(m_buffers.crbegin(), it) - 1;
    else
        return std::nullopt;
}

std::optional<BufferBindingParams> VertexArray::GetVertexBufferBinding(unsigned int index) const
{
    const auto& [buffer, binding] = m_buffers.at(index);
    return buffer ? binding : std::optional<BufferBindingParams> {};
}

void VertexArray::Apply(MTL::RenderCommandEncoder& renderEncoder)
{
    auto lastIndex = GetLastAttributeIndex();
    if (!lastIndex)
        return;

    for (size_t index = 0; index <= *lastIndex; ++index)
    {
        if (auto [buffer, binding] = m_buffers[index]; buffer != nullptr)
            renderEncoder.setVertexBuffer(Utils::safe_dereference_cast<Buffer&>(buffer).getNativeHandle(), binding.Offset, index);
    }
}