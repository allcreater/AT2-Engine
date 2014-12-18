#include "GlVertexArray.h"
#include <map>

using namespace AT2;

GlVertexArray::GlVertexArray(IRendererCapabilities* _rendererCapabilities) :
	m_buffers(_rendererCapabilities->GetMaxNumberOfVertexAttributes())
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

void GlVertexArray::SetIndexBuffer(const std::shared_ptr<GlVertexBufferBase>& _buffer)
{
	if (_buffer->GetType() != GlVertexBufferBase::GlBufferType::ElementArrayBuffer)
		throw AT2Exception("GlVertexBuffer: trying to set not index buffer as index buffer");

	m_indexBuffer = _buffer;
}

std::shared_ptr<GlVertexBufferBase> GlVertexArray::GetIndexBuffer() const
{
	return m_indexBuffer;
}

void GlVertexArray::SetVertexBuffer(unsigned int _index, const std::shared_ptr<GlVertexBufferBase>& _buffer)
{
	if (_buffer->GetType() == GlVertexBufferBase::GlBufferType::ElementArrayBuffer)
		throw AT2Exception("GlVertexBuffer: trying to set index buffer as attribute buffer");

	m_buffers.at(_index) = _buffer;

	if (_buffer)
	{
		glVertexArrayVertexAttribOffsetEXT(m_id, _buffer->GetId(), _index, static_cast<GLint>(_buffer->ElementTypeInfo.VectorLength), static_cast<GLenum>(_buffer->ElementTypeInfo.DataType), _buffer->ElementTypeInfo.IsNormalized ? GL_TRUE : GL_FALSE, _buffer->ElementTypeInfo.Stride, 0);
		glEnableVertexArrayAttribEXT(m_id, _index);
	}
	else
	{
		glDisableVertexArrayAttribEXT(m_id, _index);
	}
}
std::shared_ptr<GlVertexBufferBase> GlVertexArray::GetVertexBuffer(unsigned int _index) const
{
	return m_buffers.at(_index);
}
