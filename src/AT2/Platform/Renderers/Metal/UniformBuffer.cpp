#include "UniformBuffer.h"

using namespace AT2;
using namespace AT2::Metal;

UniformBuffer::UniformBuffer() : VertexBuffer(VertexBufferType::UniformBuffer)
{
	
}

void UniformBuffer::SetUniform(std::string_view name, const Uniform& value)
{
	
}

void UniformBuffer::SetUniform(std::string_view name, const std::shared_ptr<ITexture>& value)
{
	
}

void UniformBuffer::Bind(IStateManager& stateManager) const
{
	
}
