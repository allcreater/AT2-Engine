#include "VertexArray.h"

using namespace AT2::Metal;

VertexArray::VertexArray(const IRendererCapabilities& rendererCapabilities)
{
	
}

VertexArray::~VertexArray()
{
	
}

void VertexArray::Bind()
{
	
}

void VertexArray::SetIndexBuffer(std::shared_ptr<IVertexBuffer> buffer, BufferDataType type)
{
	
}

void VertexArray::SetAttributeBinding(unsigned int attributeIndex, std::shared_ptr<IVertexBuffer> buffer,
                                      const BufferBindingParams& binding)
{
}

std::shared_ptr<IVertexBuffer> VertexArray::GetVertexBuffer(unsigned int index) const
{
    return std::shared_ptr<IVertexBuffer>();
}

std::optional<BufferBindingParams> VertexArray::GetVertexBufferBinding(unsigned int index) const
{
    return std::optional<BufferBindingParams>();
}
