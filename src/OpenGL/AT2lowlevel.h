#ifndef AT2_OPENGL_LOWLEVEL_HEADER
#define AT2_OPENGL_LOWLEVEL_HEADER

#include <GL\glew.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include <SDL.h>

#include <sstream>
#include <vector>

#include "..\AT2.h"


namespace AT2
{
	class IUniformBuffer
	{
	public:
		//doubles
		virtual void SetUniform(const str& name, const GLdouble& value) = 0;
		virtual void SetUniform(const str& name, const glm::dvec2& value) = 0;
		virtual void SetUniform(const str& name, const glm::dvec3& value) = 0;
		virtual void SetUniform(const str& name, const glm::dvec4& value) = 0;

		virtual void SetUniform(const str& name, const glm::dmat2& value) = 0;
		virtual void SetUniform(const str& name, const glm::dmat3& value) = 0;
		virtual void SetUniform(const str& name, const glm::dmat4& value) = 0;

		//floats
		virtual void SetUniform(const str& name, const GLfloat& value) = 0;
		virtual void SetUniform(const str& name, const glm::vec2& value) = 0;
		virtual void SetUniform(const str& name, const glm::vec3& value) = 0;
		virtual void SetUniform(const str& name, const glm::vec4& value) = 0;

		virtual void SetUniform(const str& name, const glm::mat2& value) = 0;
		virtual void SetUniform(const str& name, const glm::mat3& value) = 0;
		virtual void SetUniform(const str& name, const glm::mat4& value) = 0;

		//integers
		virtual void SetUniform(const str& name, const GLint& value) = 0;
		virtual void SetUniform(const str& name, const glm::ivec2& value) = 0;
		virtual void SetUniform(const str& name, const glm::ivec3& value) = 0;
		virtual void SetUniform(const str& name, const glm::ivec4& value) = 0;

		virtual void Bind() = 0;
	};
}

#endif