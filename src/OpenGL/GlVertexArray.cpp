#include "GlVertexArray.h"
#include <map>

using namespace AT2;

GlVertexArray::GlVertexArray()
{
	glGenVertexArrays(1, &m_id);

	GLint maxAttribs;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttribs);
	m_buffers.resize(maxAttribs);
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

void GlVertexArray::SetIndexBuffer(const std::shared_ptr<GlVertexBufferBase>& buffer)
{
	if (buffer->GetType() != GlVertexBufferBase::GlBufferType::ElementArrayBuffer)
		throw AT2Exception("GlVertexBuffer: trying to set not index buffer as index buffer");

	m_indexBuffer = buffer;
}

std::shared_ptr<GlVertexBufferBase> GlVertexArray::GetIndexBuffer() const
{
	return m_indexBuffer;
}

void GlVertexArray::SetVertexBuffer(unsigned int index, const std::shared_ptr<GlVertexBufferBase>& buffer)
{
	if (buffer->GetType() == GlVertexBufferBase::GlBufferType::ElementArrayBuffer)
		throw AT2Exception("GlVertexBuffer: trying to set index buffer as attribute buffer");

	m_buffers.at(index) = buffer;

	if (buffer)
	{
		glVertexArrayVertexAttribOffsetEXT(m_id, buffer->GetId(), index, static_cast<GLint>(buffer->ElementTypeInfo.VectorLength), static_cast<GLenum>(buffer->ElementTypeInfo.DataType), buffer->ElementTypeInfo.IsNormalized ? GL_TRUE : GL_FALSE, buffer->ElementTypeInfo.Stride, 0);
		glEnableVertexArrayAttribEXT(m_id, index);
	}
	else
	{
		glDisableVertexArrayAttribEXT(m_id, index);
	}
}
std::shared_ptr<GlVertexBufferBase> GlVertexArray::GetVertexBuffer(unsigned int index) const
{
	return m_buffers.at(index);
}
