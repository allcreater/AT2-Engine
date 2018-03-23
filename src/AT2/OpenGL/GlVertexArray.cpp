#include "GlVertexArray.h"
#include <map>

using namespace AT2;

GlVertexArray::GlVertexArray(const IRendererCapabilities& _rendererCapabilities) :
	m_buffers(_rendererCapabilities.GetMaxNumberOfVertexAttributes())
{
	glGenVertexArrays(1, &m_id);
}

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

void GlVertexArray::SetIndexBuffer(const std::shared_ptr<IVertexBuffer>& _buffer)
{
	if (_buffer) //if present we must ensure that it right
	{
		assert(dynamic_cast<GlVertexBufferBase*>(_buffer.get()));

		if (_buffer->GetType() != VertexBufferType::IndexBuffer)
			throw AT2Exception("GlVertexBuffer: must be index buffer!");
	}

	m_indexBuffer = _buffer;
}

std::shared_ptr<IVertexBuffer> GlVertexArray::GetIndexBuffer() const
{
	return m_indexBuffer;
}

void GlVertexArray::SetVertexBuffer(unsigned int _index, const std::shared_ptr<IVertexBuffer>& _buffer)
{
	m_buffers.at(_index) = _buffer;

	if (_buffer)
	{
		assert(dynamic_cast<GlVertexBufferBase*>(_buffer.get()));

		if (_buffer->GetType() != VertexBufferType::ArrayBuffer)
			throw AT2Exception("GlVertexBuffer: trying to attach incorrect type buffer");

		auto dataType = reinterpret_cast<const GlVertexBufferBase::GlBufferTypeInfo&>(_buffer->GetDataType());

		glVertexArrayVertexAttribOffsetEXT(m_id, _buffer->GetId(), _index, static_cast<GLint>(dataType.Count), static_cast<GLenum>(dataType.GlDataType), dataType.IsNormalized ? GL_TRUE : GL_FALSE, dataType.Stride, 0);
		glEnableVertexArrayAttribEXT(m_id, _index);
	}
	else
	{
		glDisableVertexArrayAttribEXT(m_id, _index);
	}
}
std::shared_ptr<IVertexBuffer> GlVertexArray::GetVertexBuffer(unsigned int _index) const
{
	return m_buffers.at(_index);
}
