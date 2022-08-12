#include "VertexArray.h"
#include "Mappings.h"

using namespace AT2;
using namespace Metal;

VertexArray::VertexArray(const IRendererCapabilities& rendererCapabilities)
: m_buffers(rendererCapabilities.GetMaxNumberOfVertexAttributes())
{
    m_vertexDescriptor = ConstructMetalObject<MTL::VertexDescriptor>();
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
    //const auto& mtlBuffer = Utils::safe_dereference_cast<Buffer&>(buffer.get());

    const auto bindingIndex = attributeIndex;

    auto* attribute = m_vertexDescriptor->attributes()->object(attributeIndex);
    attribute->setFormat(Mappings::TranslateVertexFormat(binding));
    //attribute->setOffset(binding.Offset); //this could be useful only when multiple attibutes could binds to one buffer. We have 1 to 1 mapping, so that dynamic offsets are more flexible
    attribute->setBufferIndex(bindingIndex); //TODO:
    
    auto layout = m_vertexDescriptor->layouts()->object(bindingIndex);
    layout->setStride(binding.Stride);
    
    if (binding.Divisor > 0)
    {
        layout->setStepRate(binding.Divisor);
        layout->setStepFunction(MTL::VertexStepFunctionPerInstance);
    }
    else
    {
        layout->setStepRate(1);
        layout->setStepFunction(MTL::VertexStepFunctionPerVertex);
    }
    
    
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