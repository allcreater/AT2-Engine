#include "GlShaderProgram.h"

using namespace AT2;

GlShaderProgram::GlShaderProgram(const str& _vs, const str& _tcs, const str& _tes, const str& _gs, const str& _fs)
{
	m_programId = glCreateProgram();

	if (!_vs.empty())
	{
		m_shaderId[0] = LoadShader(GL_VERTEX_SHADER, _vs);
		glAttachShader(m_programId, m_shaderId[0]);
	}
	else
		throw AT2Exception(AT2::AT2Exception::ErrorCase::Shader, "GlShaderProgram: empty vertex shader");

	if (!_tcs.empty())
	{
		m_shaderId[1] = LoadShader(GL_TESS_CONTROL_SHADER, _tcs);
		glAttachShader(m_programId, m_shaderId[1]);
	}

	if (!_tes.empty())
	{
		m_shaderId[2] = LoadShader(GL_TESS_EVALUATION_SHADER, _tes);
		glAttachShader(m_programId, m_shaderId[2]);
	}

	if (!_tes.empty())
	{
		m_shaderId[3] = LoadShader(GL_GEOMETRY_SHADER, _gs);
		glAttachShader(m_programId, m_shaderId[3]);
	}

	if (!_fs.empty())
	{
		m_shaderId[4] = LoadShader(GL_FRAGMENT_SHADER, _fs);
		glAttachShader(m_programId, m_shaderId[4]);
	}

	glLinkProgram (m_programId);
	GLint linked = 0;
	glGetProgramiv  (m_programId, GL_LINK_STATUS, &linked );

	//log
	GLchar infoLogBuffer[2048];
	GLint infoLogLength;
	glGetProgramInfoLog(m_programId, 2048, &infoLogLength, infoLogBuffer);
	if (infoLogLength > 0)
		Log::Debug() << "Shader program log: " << std::endl << infoLogBuffer;


	if (!linked)
		throw AT2Exception(AT2::AT2Exception::ErrorCase::Shader, "GlShaderProgram: program not linked");
}

GlShaderProgram::~GlShaderProgram()
{
	glDeleteProgram(m_programId);

	for (int i = 0; i < 5; ++i)
		if (m_shaderId[i] != 0)
			glDeleteShader(m_shaderId[i]);
}

GLuint GlShaderProgram::LoadShader(GLenum _shaderType, const str& _text)
{
	GLuint shader = glCreateShader(_shaderType);

	const GLchar* data = _text.c_str();
	GLint dataLength = _text.length();
	glShaderSource(shader, 1, &data, &dataLength);

	glCompileShader(shader);


	GLchar infoLogBuffer[2048];
	GLint infoLogLength;
	glGetShaderInfoLog(shader, 2048, &infoLogLength, infoLogBuffer);
	if (infoLogLength > 0)
		Log::Debug() << "Shader log: " << std::endl << infoLogBuffer;

	GLint status = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE)
	{
		std::stringstream ss;
		ss << "GlShaderProgram: shader compilation failed!" << std::endl << infoLogBuffer;

		throw AT2Exception(AT2::AT2Exception::ErrorCase::Shader, ss.str());
	}

	return shader;
}

void GlShaderProgram::Bind()
{
	glUseProgram(m_programId);
}

//WARNING: copypaste!
void GlShaderProgram::SetUniform(const str& _name, GLfloat _value)
{
	GLint location = glGetUniformLocation(m_programId, _name.c_str());
	if (location >= 0)
		glProgramUniform1f(m_programId, location, _value);
}
void GlShaderProgram::SetUniform(const str& _name, const glm::vec2& _value)
{
	GLint location = glGetUniformLocation(m_programId, _name.c_str());
	if (location >= 0)
		glProgramUniform2fv(m_programId, location, 1, glm::value_ptr(_value));
}
void GlShaderProgram::SetUniform(const str& _name, const glm::vec3& _value)
{
	GLint location = glGetUniformLocation(m_programId, _name.c_str());
	if (location >= 0)
		glProgramUniform3fv(m_programId, location, 1, glm::value_ptr(_value));
}
void GlShaderProgram::SetUniform(const str& _name, const glm::vec4& _value)
{
	GLint location = glGetUniformLocation(m_programId, _name.c_str());
	if (location >= 0)
		glProgramUniform4fv(m_programId, location, 1, glm::value_ptr(_value));
}
void GlShaderProgram::SetUniform(const str& _name, GLint _value)
{
	GLint location = glGetUniformLocation(m_programId, _name.c_str());
	if (location >= 0)
		glProgramUniform1i(m_programId, location, _value);
}
void GlShaderProgram::SetUniform(const str& _name, const glm::ivec2& _value)
{
	GLint location = glGetUniformLocation(m_programId, _name.c_str());
	if (location >= 0)
		glProgramUniform2iv(m_programId, location, 1, glm::value_ptr(_value));
}
void GlShaderProgram::SetUniform(const str& _name, const glm::ivec3& _value)
{
	GLint location = glGetUniformLocation(m_programId, _name.c_str());
	if (location >= 0)
		glProgramUniform3iv(m_programId, location, 1, glm::value_ptr(_value));
}
void GlShaderProgram::SetUniform(const str& _name, const glm::ivec4& _value)
{
	GLint location = glGetUniformLocation(m_programId, _name.c_str());
	if (location >= 0)
		glProgramUniform4iv(m_programId, location, 1, glm::value_ptr(_value));
}
void GlShaderProgram::SetUniform(const str& _name, const glm::mat2& _value)
{
	GLint location = glGetUniformLocation(m_programId, _name.c_str());
	if (location >= 0)
		glProgramUniformMatrix2fv(m_programId, location, 1, GL_FALSE, glm::value_ptr(_value));
}
void GlShaderProgram::SetUniform(const str& _name, const glm::mat3& _value)
{
	GLint location = glGetUniformLocation(m_programId, _name.c_str());
	if (location >= 0)
		glProgramUniformMatrix3fv(m_programId, location, 1, GL_FALSE, glm::value_ptr(_value));
}
void GlShaderProgram::SetUniform(const str& _name, const glm::mat4& _value)
{
	GLint location = glGetUniformLocation(m_programId, _name.c_str());
	if (location >= 0)
		glProgramUniformMatrix4fv(m_programId, location, 1, GL_FALSE, glm::value_ptr(_value));
}