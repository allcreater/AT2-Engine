#include "GlUniformContainer.h"

using namespace AT2;
using namespace glm;

inline const GLint* value_ptr(const GLint& value) { return &value; }
inline const GLfloat* value_ptr(const GLfloat& value) { return &value; }
inline const GLboolean* value_ptr(const GLboolean& value) { return &value; }

template <typename T>
class GlUniform : public IGlUniform
{
};

//doubles
template <>
class GlUniform<GLdouble> : public IGlUniform
{
public:
	void set(const GLdouble& value)	override	{ m_value = value; }

	void Bind() override						{ glUniform1d(GetLocation(), m_value); }
	void Bind(GLuint programID) override		{ glProgramUniform1d(programID, GetLocation(), m_value); }
	
	GlUniform() : IGlUniform() {}

private:
	GLdouble m_value {};
};

template <>
class GlUniform<dvec2> : public IGlUniform
{
public:
	void set(const glm::dvec2& value) override	{ m_value = value; }

	void Bind() override						{ glUniform2dv(GetLocation(), 1, glm::value_ptr(m_value)); }
	void Bind(GLuint programID) override		{ glProgramUniform2dv(programID, 1, GetLocation(), glm::value_ptr(m_value)); }

	GlUniform() : IGlUniform() {}

private:
	glm::dvec2 m_value {};
};

template <>
class GlUniform<dvec3> : public IGlUniform
{
public:
	void set(const glm::dvec3& value) override	{ m_value = value; }

	void Bind() override						{ glUniform3dv(GetLocation(), 1, glm::value_ptr(m_value)); }
	void Bind(GLuint programID) override		{ glProgramUniform3dv(programID, 1, GetLocation(), glm::value_ptr(m_value)); }

	GlUniform() : IGlUniform() {}

private:
	glm::dvec3 m_value {};
};

template <>
class GlUniform<dvec4> : public IGlUniform
{
public:
	void set(const glm::dvec4& value) override	{ m_value = value; }

	void Bind() override						{ glUniform4dv(GetLocation(), 1, glm::value_ptr(m_value)); }
	void Bind(GLuint programID) override		{ glProgramUniform4dv(programID, GetLocation(), 1, glm::value_ptr(m_value)); }

	GlUniform() : IGlUniform() {}

private:
	glm::dvec4 m_value {};
};

template <>
class GlUniform<dmat2> : public IGlUniform
{
public:
	void set(const glm::dmat2& value) override	{ m_value = value; }

	void Bind() override						{ glUniformMatrix2dv(GetLocation(), 1, GL_FALSE, glm::value_ptr(m_value)); }
	void Bind(GLuint programID) override		{ glProgramUniformMatrix2dv(programID, GetLocation(), 1, GL_FALSE, glm::value_ptr(m_value)); }

	GlUniform() : IGlUniform() {}

private:
	glm::dmat2 m_value {};
};

template <>
class GlUniform<dmat3> : public IGlUniform
{
public:
	void set(const glm::dmat3& value) override	{ m_value = value; }

	void Bind() override						{ glUniformMatrix3dv(GetLocation(), 1, GL_FALSE, glm::value_ptr(m_value)); }
	void Bind(GLuint programID) override		{ glProgramUniformMatrix3dv(programID, GetLocation(), 1, GL_FALSE, glm::value_ptr(m_value)); }

	GlUniform() : IGlUniform() {}

private:
	glm::dmat3 m_value {};
};

template <>
class GlUniform<dmat4> : public IGlUniform
{
public:
	void set(const glm::dmat4& value) override	{ m_value = value; }

	void Bind() override						{ glUniformMatrix4dv(GetLocation(), 1, GL_FALSE, glm::value_ptr(m_value)); }
	void Bind(GLuint programID) override		{ glProgramUniformMatrix4dv(programID, GetLocation(), 1, GL_FALSE, glm::value_ptr(m_value)); }

	GlUniform() : IGlUniform() {}

private:
	glm::dmat4 m_value {};
};

//floats
template <>
class GlUniform<GLfloat> : public IGlUniform
{
public:
	void set(const GLfloat& value) override		{ m_value = value; }

	void Bind() override						{ glUniform1f(GetLocation(), m_value); }
	void Bind(GLuint programID) override		{ glProgramUniform1f(programID, GetLocation(), m_value); }

	GlUniform() : IGlUniform() {}

private:
	GLfloat m_value {};
};

template <>
class GlUniform<vec2> : public IGlUniform
{
public:
	void set(const glm::vec2& value) override	{ m_value = value; }

	void Bind() override						{ glUniform2fv(GetLocation(), 1, glm::value_ptr(m_value)); }
	void Bind(GLuint programID) override		{ glProgramUniform2fv(programID, 1, GetLocation(), glm::value_ptr(m_value)); }

	GlUniform() : IGlUniform() {}

private:
	glm::vec2 m_value {};
};

template <>
class GlUniform<vec3> : public IGlUniform
{
public:
	void set(const glm::vec3& value) override	{ m_value = value; }

	void Bind() override						{ glUniform3fv(GetLocation(), 1, glm::value_ptr(m_value)); }
	void Bind(GLuint programID) override		{ glProgramUniform3fv(programID, 1, GetLocation(), glm::value_ptr(m_value)); }

	GlUniform() : IGlUniform() {}

private:
	glm::vec3 m_value {};
};

template <>
class GlUniform<vec4> : public IGlUniform
{
public:
	void set(const glm::vec4& value) override	{ m_value = value; }

	void Bind() override						{ glUniform4fv(GetLocation(), 1, glm::value_ptr(m_value)); }
	void Bind(GLuint programID) override		{ glProgramUniform4fv(programID, GetLocation(), 1, glm::value_ptr(m_value)); }

	GlUniform() : IGlUniform() {}

private:
	glm::vec4 m_value {};
};

template <>
class GlUniform<mat2> : public IGlUniform
{
public:
	void set(const glm::mat2& value) override	{ m_value = value; }

	void Bind() override						{ glUniformMatrix2fv(GetLocation(), 1, GL_FALSE, glm::value_ptr(m_value)); }
	void Bind(GLuint programID) override		{ glProgramUniformMatrix2fv(programID, GetLocation(), 1, GL_FALSE, glm::value_ptr(m_value)); }

	GlUniform() : IGlUniform() {}

private:
	glm::mat2 m_value {};
};

template <>
class GlUniform<mat3> : public IGlUniform
{
public:
	void set(const glm::mat3& value) override	{ m_value = value; }

	void Bind() override						{ glUniformMatrix3fv(GetLocation(), 1, GL_FALSE, glm::value_ptr(m_value)); }
	void Bind(GLuint programID) override		{ glProgramUniformMatrix3fv(programID, GetLocation(), 1, GL_FALSE, glm::value_ptr(m_value)); }

	GlUniform() : IGlUniform() {}

private:
	glm::mat3 m_value {};
};

template <>
class GlUniform<mat4> : public IGlUniform
{
public:
	void set(const glm::mat4& value) override	{ m_value = value; }

	void Bind() override						{ glUniformMatrix4fv(GetLocation(), 1, GL_FALSE, glm::value_ptr(m_value)); }
	void Bind(GLuint programID) override		{ glProgramUniformMatrix4fv(programID, GetLocation(), 1, GL_FALSE, glm::value_ptr(m_value)); }

	GlUniform() : IGlUniform() {}

private:
	glm::mat4 m_value {};
};

//integers
template <>
class GlUniform<GLint> : public IGlUniform
{
public:
	void set(const GLint& value) override		{ m_value = value; }

	void Bind() override						{ glUniform1i(GetLocation(), m_value); }
	void Bind(GLuint programID) override		{ glProgramUniform1i(programID, GetLocation(), m_value); }

	GlUniform() : IGlUniform() {}

private:
	GLint m_value {};
};

template <>
class GlUniform<ivec2> : public IGlUniform
{
public:
	void set(const glm::ivec2& value) override	{ m_value = value; }

	void Bind() override						{ glUniform2iv(GetLocation(), 1, glm::value_ptr(m_value)); }
	void Bind(GLuint programID) override		{ glProgramUniform2iv(programID, 1, GetLocation(), glm::value_ptr(m_value)); }

	GlUniform() : IGlUniform() {}

private:
	glm::ivec2 m_value {};
};

template <>
class GlUniform<ivec3> : public IGlUniform
{
public:
	void set(const glm::ivec3& value) override	{ m_value = value; }

	void Bind() override						{ glUniform3iv(GetLocation(), 1, glm::value_ptr(m_value)); }
	void Bind(GLuint programID) override		{ glProgramUniform3iv(programID, 1, GetLocation(), glm::value_ptr(m_value)); }

	GlUniform() : IGlUniform() {}

private:
	glm::ivec3 m_value {};
};

template <>
class GlUniform<ivec4> : public IGlUniform
{
public:
	void set(const glm::ivec4& value) override	{ m_value = value; }

	void Bind() override						{ glUniform4iv(GetLocation(), 1, glm::value_ptr(m_value)); }
	void Bind(GLuint programID) override		{ glProgramUniform4iv(programID, GetLocation(), 1, glm::value_ptr(m_value)); }

	GlUniform() : IGlUniform() {}

private:
	glm::ivec4 m_value {};
};

//unsigned integers
template <>
class GlUniform<GLuint> : public IGlUniform
{
public:
	void set(const GLuint& value) override { m_value = value; }

	void Bind() override { glUniform1ui(GetLocation(), m_value); }
	void Bind(GLuint programID) override { glProgramUniform1ui(programID, GetLocation(), m_value); }

	GlUniform() : IGlUniform() {}

private:
	GLuint m_value {};
};

template <>
class GlUniform<uvec2> : public IGlUniform
{
public:
	void set(const glm::uvec2& value) override { m_value = value; }

	void Bind() override { glUniform2uiv(GetLocation(), 1, glm::value_ptr(m_value)); }
	void Bind(GLuint programID) override { glProgramUniform2uiv(programID, 1, GetLocation(), glm::value_ptr(m_value)); }

	GlUniform() : IGlUniform() {}

private:
	glm::uvec2 m_value {};
};

template <>
class GlUniform<uvec3> : public IGlUniform
{
public:
	void set(const glm::uvec3& value) override { m_value = value; }

	void Bind() override { glUniform3uiv(GetLocation(), 1, glm::value_ptr(m_value)); }
	void Bind(GLuint programID) override { glProgramUniform3uiv(programID, 1, GetLocation(), glm::value_ptr(m_value)); }

	GlUniform() : IGlUniform() {}

private:
	glm::uvec3 m_value {};
};

template <>
class GlUniform<uvec4> : public IGlUniform
{
public:
	void set(const glm::uvec4& value) override { m_value = value; }

	void Bind() override { glUniform4uiv(GetLocation(), 1, glm::value_ptr(m_value)); }
	void Bind(GLuint programID) override { glProgramUniform4uiv(programID, GetLocation(), 1, glm::value_ptr(m_value)); }

	GlUniform() : IGlUniform() {}

private:
	glm::uvec4 m_value {};
};

template <>
class GlUniform<std::weak_ptr<const ITexture>> : public IGlUniform
{
public:
	void set(std::weak_ptr<const ITexture> value) override	{ m_value = value; }

	void Bind() override					
	{
		if (auto texture = m_value.lock())
			glUniform1i(GetLocation(), texture->GetCurrentModule());
	}
	void Bind(GLuint programID) override	
	{
		if (auto texture = m_value.lock())
			glProgramUniform1i(programID, GetLocation(), texture->GetCurrentModule());
	}

	GlUniform() : IGlUniform() {}

private:
	std::weak_ptr<const ITexture> m_value {};
};

//
// GlUniformContainer
//

GlUniformContainer::GlUniformContainer(std::weak_ptr<GlShaderProgram> _program)
{
	SetProgram(_program);
}

GlUniformContainer::~GlUniformContainer()
{
}

template <typename T>
IGlUniform* GlUniformContainer::GetOrCreateUniform(const str& _name)
{
	auto& value = m_uniformsMap[_name];
	if (value == nullptr)
		value = new GlUniform<T>();

	return value;
}

void GlUniformContainer::SetUniform(const str& name, const GLdouble& value)		{ GetOrCreateUniform<GLdouble>(name)->set(value); }
void GlUniformContainer::SetUniform(const str& name, const glm::dvec2& value)	{ GetOrCreateUniform<dvec2>(name)->set(value); }
void GlUniformContainer::SetUniform(const str& name, const glm::dvec3& value)	{ GetOrCreateUniform<dvec3>(name)->set(value); }
void GlUniformContainer::SetUniform(const str& name, const glm::dvec4& value)	{ GetOrCreateUniform<dvec4>(name)->set(value); }

void GlUniformContainer::SetUniform(const str& name, const glm::dmat2& value)	{ GetOrCreateUniform<dmat2>(name)->set(value); }
void GlUniformContainer::SetUniform(const str& name, const glm::dmat3& value)	{ GetOrCreateUniform<dmat3>(name)->set(value); }
void GlUniformContainer::SetUniform(const str& name, const glm::dmat4& value)	{ GetOrCreateUniform<dmat4>(name)->set(value); }

//floats
void GlUniformContainer::SetUniform(const str& name, const GLfloat& value)		{ GetOrCreateUniform<GLfloat>(name)->set(value); }
void GlUniformContainer::SetUniform(const str& name, const glm::vec2& value)	{ GetOrCreateUniform<vec2>(name)->set(value); }
void GlUniformContainer::SetUniform(const str& name, const glm::vec3& value)	{ GetOrCreateUniform<vec3>(name)->set(value); }
void GlUniformContainer::SetUniform(const str& name, const glm::vec4& value)	{ GetOrCreateUniform<vec4>(name)->set(value); }

void GlUniformContainer::SetUniform(const str& name, const glm::mat2& value)	{ GetOrCreateUniform<mat2>(name)->set(value); }
void GlUniformContainer::SetUniform(const str& name, const glm::mat3& value)	{ GetOrCreateUniform<mat3>(name)->set(value); }
void GlUniformContainer::SetUniform(const str& name, const glm::mat4& value)	{ GetOrCreateUniform<mat4>(name)->set(value); }

//integers	
void GlUniformContainer::SetUniform(const str& name, const GLint& value)		{ GetOrCreateUniform<GLint>(name)->set(value); }
void GlUniformContainer::SetUniform(const str& name, const glm::ivec2& value)	{ GetOrCreateUniform<ivec2>(name)->set(value); }
void GlUniformContainer::SetUniform(const str& name, const glm::ivec3& value)	{ GetOrCreateUniform<ivec3>(name)->set(value); }
void GlUniformContainer::SetUniform(const str& name, const glm::ivec4& value)	{ GetOrCreateUniform<ivec4>(name)->set(value); }

//unsigned integers	
void GlUniformContainer::SetUniform(const str& name, const glm::uint&  value)	{ GetOrCreateUniform<GLuint>(name)->set(value); }
void GlUniformContainer::SetUniform(const str& name, const glm::uvec2& value)	{ GetOrCreateUniform<uvec2> (name)->set(value); }
void GlUniformContainer::SetUniform(const str& name, const glm::uvec3& value)	{ GetOrCreateUniform<uvec3> (name)->set(value); }
void GlUniformContainer::SetUniform(const str& name, const glm::uvec4& value)	{ GetOrCreateUniform<uvec4> (name)->set(value); }

//texture
void GlUniformContainer::SetUniform(const str& name, std::weak_ptr<const ITexture> value)	{ GetOrCreateUniform<std::weak_ptr<const ITexture>>(name)->set(value); }

//TODO: maybe it's better to detach uniform container from specific program at all?
void GlUniformContainer::Bind()
{
	auto program = m_program.lock();
	if (!program)
		throw AT2::AT2Exception(AT2Exception::ErrorCase::Shader, "GlUniformContainer: try to bind uniform container when parent program is missing");

	
	if (program->IsActive())
	{
		for (auto& uniform : m_uniformsMap)
		{
			uniform.second->m_location = glGetUniformLocation(program->GetId(), uniform.first.c_str());
			uniform.second->Bind();
		}
	}
	else
	{
		for (auto& uniform : m_uniformsMap)
		{
			uniform.second->m_location = glGetUniformLocation(program->GetId(), uniform.first.c_str());
			uniform.second->Bind(program->GetId());
		}
	}
}