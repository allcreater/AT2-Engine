#include "GlUniformBuffer.h"

using namespace AT2;

GlUniformBuffer::GlUniformBuffer(std::shared_ptr<GlShaderProgram::UniformBufferInfo> ubi) : 
	m_bindingPoint(0),
	m_ubi(ubi)
{
	UsageHint = GlBufferUsageHint::StreamDraw;

	glGenBuffers(1, &m_id);
	glNamedBufferDataEXT(m_id, m_ubi->GetBlockSize(), 0, static_cast<GLenum>(UsageHint));
}

GlUniformBuffer::~GlUniformBuffer()
{
	glDeleteBuffers(1, &m_id);
}

void GlUniformBuffer::Bind()
{
	glBindBufferBase(GL_UNIFORM_BUFFER, m_bindingPoint, m_id);
}

/*
void GlUniformBuffer::SetUniform(const str& name, glm::vec4 value)
{
	const GlShaderProgram::UniformInfo* ui = m_ubi->GetUniformInfo(name);
	if (!ui)
		return;

	auto data = reinterpret_cast<GLbyte*>(glMapNamedBufferEXT(m_id, static_cast<GLenum>(UsageHint)));
	memcpy(data + ui->Offset, glm::value_ptr(value), sizeof(value));
	glUnmapNamedBufferEXT(m_id);
}
*/