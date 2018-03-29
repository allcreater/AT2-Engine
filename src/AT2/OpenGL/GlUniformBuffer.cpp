#include "GlUniformBuffer.h"

using namespace AT2;

using namespace glm;

GlUniformBuffer::GlUniformBuffer(std::shared_ptr<GlShaderProgram::UniformBufferInfo> ubi) : 
	GlVertexBuffer (VertexBufferType::UniformBuffer),
	m_bindingPoint(0),
	m_ubi(ubi)
{
	m_length = m_ubi->GetBlockSize();

	glNamedBufferDataEXT(m_id, m_length, 0, static_cast<GLenum>(m_usageHint));
}

GlUniformBuffer::~GlUniformBuffer()
{
	glDeleteBuffers(1, &m_id);
}

void GlUniformBuffer::Bind()
{
	glBindBufferBase(GL_UNIFORM_BUFFER, m_bindingPoint, m_id);
}

template <typename T>
const GLvoid* value_ptr(const T& value) { return reinterpret_cast<const GLvoid*>(&value); }

template <typename T>
void GlUniformBuffer::SetUniformInternal(const str& name, const T& value, size_t size)
{
	const GlShaderProgram::UniformInfo* ui = m_ubi->GetUniformInfo(name);
	if (!ui)
		return;

	auto data = reinterpret_cast<GLbyte*>(glMapNamedBufferEXT(m_id, GL_WRITE_ONLY));
	const void* ptr = value_ptr(value);
	memcpy(data + ui->Offset, ptr, size);
	glUnmapNamedBufferEXT(m_id);
}


//mat2
template <> void GlUniformBuffer::SetUniformInternal<>(const str& name, const mat2& value, size_t size)
{
	const GlShaderProgram::UniformInfo* ui = m_ubi->GetUniformInfo(name);
	if (!ui)
		return;

	auto data = reinterpret_cast<GLbyte*>(glMapNamedBufferEXT(m_id, GL_WRITE_ONLY));
	memcpy(data + ui->Offset, &(value[0].x), sizeof(mat2::col_type));
	memcpy(data + ui->Offset + ui->MatrixStride, &(value[1].x), sizeof(mat2::col_type));
	glUnmapNamedBufferEXT(m_id);
}
template <> void GlUniformBuffer::SetUniformInternal<>(const str& name, const dmat2& value, size_t size)
{
	const GlShaderProgram::UniformInfo* ui = m_ubi->GetUniformInfo(name);
	if (!ui)
		return;

	auto data = reinterpret_cast<GLbyte*>(glMapNamedBufferEXT(m_id, GL_WRITE_ONLY));
	memcpy(data + ui->Offset, &(value[0].x), sizeof(dmat2::col_type));
	memcpy(data + ui->Offset + ui->MatrixStride, &(value[1].x), sizeof(dmat2::col_type));
	glUnmapNamedBufferEXT(m_id);
}
//mat3
template <> void GlUniformBuffer::SetUniformInternal<>(const str& name, const mat3& value, size_t size)
{
	const GlShaderProgram::UniformInfo* ui = m_ubi->GetUniformInfo(name);
	if (!ui)
		return;

	auto data = reinterpret_cast<GLbyte*>(glMapNamedBufferEXT(m_id, GL_WRITE_ONLY));
	memcpy(data + ui->Offset, &(value[0].x), sizeof(mat3::col_type));
	memcpy(data + ui->Offset + ui->MatrixStride, &(value[1].x), sizeof(mat3::col_type));
	memcpy(data + ui->Offset + ui->MatrixStride * 2, &(value[2].x), sizeof(mat3::col_type));
	glUnmapNamedBufferEXT(m_id);
}
template <> void GlUniformBuffer::SetUniformInternal<>(const str& name, const dmat3& value, size_t size)
{
	const GlShaderProgram::UniformInfo* ui = m_ubi->GetUniformInfo(name);
	if (!ui)
		return;

	auto data = reinterpret_cast<GLbyte*>(glMapNamedBufferEXT(m_id, GL_WRITE_ONLY));
	memcpy(data + ui->Offset, &(value[0].x), sizeof(dmat3::col_type));
	memcpy(data + ui->Offset + ui->MatrixStride, &(value[1].x), sizeof(dmat3::col_type));
	memcpy(data + ui->Offset + ui->MatrixStride * 2, &(value[2].x), sizeof(dmat3::col_type));
	glUnmapNamedBufferEXT(m_id);
}

//doubles
void GlUniformBuffer::SetUniform(const str& name, const GLdouble& value) { SetUniformInternal(name, value); }
void GlUniformBuffer::SetUniform(const str& name, const glm::dvec2& value) { SetUniformInternal(name, value); }
void GlUniformBuffer::SetUniform(const str& name, const glm::dvec3& value) { SetUniformInternal(name, value); }
void GlUniformBuffer::SetUniform(const str& name, const glm::dvec4& value) { SetUniformInternal(name, value); }

void GlUniformBuffer::SetUniform(const str& name, const glm::dmat2& value) { SetUniformInternal(name, value); }
void GlUniformBuffer::SetUniform(const str& name, const glm::dmat3& value) { SetUniformInternal(name, value); }
void GlUniformBuffer::SetUniform(const str& name, const glm::dmat4& value) { SetUniformInternal(name, value); }

//floats
void GlUniformBuffer::SetUniform(const str& name, const GLfloat& value) { SetUniformInternal(name, value); }
void GlUniformBuffer::SetUniform(const str& name, const glm::vec2& value) { SetUniformInternal(name, value); }
void GlUniformBuffer::SetUniform(const str& name, const glm::vec3& value) { SetUniformInternal(name, value); }
void GlUniformBuffer::SetUniform(const str& name, const glm::vec4& value) { SetUniformInternal(name, value); }

void GlUniformBuffer::SetUniform(const str& name, const glm::mat2& value) { SetUniformInternal(name, value); }
void GlUniformBuffer::SetUniform(const str& name, const glm::mat3& value) { SetUniformInternal(name, value); }
void GlUniformBuffer::SetUniform(const str& name, const glm::mat4& value) { SetUniformInternal(name, value); }

//integers
void GlUniformBuffer::SetUniform(const str& name, const GLint& value) { SetUniformInternal(name, value); }
void GlUniformBuffer::SetUniform(const str& name, const glm::ivec2& value) { SetUniformInternal(name, value); }
void GlUniformBuffer::SetUniform(const str& name, const glm::ivec3& value) { SetUniformInternal(name, value); }
void GlUniformBuffer::SetUniform(const str& name, const glm::ivec4& value) { SetUniformInternal(name, value); }

//unsigned integers
void GlUniformBuffer::SetUniform(const str& name, const GLuint& value) { SetUniformInternal(name, value); }
void GlUniformBuffer::SetUniform(const str& name, const glm::uvec2& value) { SetUniformInternal(name, value); }
void GlUniformBuffer::SetUniform(const str& name, const glm::uvec3& value) { SetUniformInternal(name, value); }
void GlUniformBuffer::SetUniform(const str& name, const glm::uvec4& value) { SetUniformInternal(name, value); }

void GlUniformBuffer::SetUniform(const str& name, std::weak_ptr<const ITexture> value) { throw std::logic_error("not implemented"); }