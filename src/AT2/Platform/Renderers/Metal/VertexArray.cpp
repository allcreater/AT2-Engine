#include "VertexArray.h"

using namespace AT2;
using namespace Metal;

VertexArray::VertexArray(const IRendererCapabilities& rendererCapabilities)
: m_buffers(rendererCapabilities.GetMaxNumberOfVertexAttributes())
{
	
}

VertexArray::~VertexArray()
{
	
}

void VertexArray::Bind()
{
	
}

void VertexArray::SetIndexBuffer(std::shared_ptr<IBuffer> buffer, BufferDataType type)
{
	
}

void VertexArray::SetAttributeBinding(unsigned int attributeIndex, std::shared_ptr<IBuffer> buffer,
                                      const BufferBindingParams& binding)
{
    m_buffers.at(attributeIndex) = { buffer, binding };
}

std::shared_ptr<IBuffer> VertexArray::GetVertexBuffer(unsigned int index) const
{
    return m_buffers.at(index).first;
}

std::optional<BufferBindingParams> VertexArray::GetVertexBufferBinding(unsigned int index) const
{
    return std::optional<BufferBindingParams>();
}
