#include "GlUniformBuffer.h"

using namespace AT2;

GlUniformBuffer::GlUniformBuffer(const GlShaderProgram& shader, const str& blockName)
{
	m_ubi = shader.GetUniformBlockInfo(blockName);

	glGenBuffers(1, &m_id);
}

GlUniformBuffer::~GlUniformBuffer()
{
	glDeleteBuffers(1, &m_id);
}

void GlUniformBuffer::Bind()
{
	glBindBufferBase(GL_UNIFORM_BUFFER, m_ubi->GetBlockIndex(), m_id);
}