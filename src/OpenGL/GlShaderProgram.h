#ifndef AT2_GL_SHADERPROGRAM_H
#define AT2_GL_SHADERPROGRAM_H

#include "AT2lowlevel.h"

namespace AT2
{
	class GlShaderProgram : public IShaderProgram
	{
	public:
		GlShaderProgram(const str& _vs, const str& _tcs, const str& _tes, const str& _gs, const str& _fs);
		virtual ~GlShaderProgram();

		virtual void Bind();
		virtual unsigned int GetId() const { return m_programId; }

		//floats
		void SetUniform(const str& _name, GLfloat _value);
		void SetUniform(const str& _name, const glm::vec2& _value);
		void SetUniform(const str& _name, const glm::vec3& _value);
		void SetUniform(const str& _name, const glm::vec4& _value);

		//integers
		void SetUniform(const str& _name, GLint _value);
		void SetUniform(const str& _name, const glm::ivec2& _value);
		void SetUniform(const str& _name, const glm::ivec3& _value);
		void SetUniform(const str& _name, const glm::ivec4& _value);

		//matrices
		void SetUniform(const str& _name, const glm::mat2& _value);
		void SetUniform(const str& _name, const glm::mat3& _value);
		void SetUniform(const str& _name, const glm::mat4& _value);

	protected:
		GLuint LoadShader(GLenum _shaderType, const str& _text);

	private:
		GLuint m_programId;
		GLuint m_shaderId[5];
	};
}

#endif