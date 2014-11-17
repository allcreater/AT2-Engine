#ifndef AT2_GL_SHADERPROGRAM_H
#define AT2_GL_SHADERPROGRAM_H

#include "AT2lowlevel.h"

namespace AT2
{
	class GlShaderProgram : public IShaderProgram
	{
	public:
		class UniformBufferInfo
		{
		friend class GlShaderProgram;

		public:
			GLuint	GetBlockIndex() const {return m_blockIndex;}
			GLint	GetBlockSize() const {return m_blockSize; }

		private:
			GLuint m_blockIndex; 
			GLint m_blockSize;

			GLint m_numActiveUniforms;
			std::vector<GLint> m_activeUniformIndices;
		};

	public:
		GlShaderProgram(const str& vs, const str& tcs, const str& tes, const str& gs, const str& fs);
		virtual ~GlShaderProgram();

		virtual void Bind();
		virtual unsigned int GetId() const { return m_programId; }

		//floats
		void SetUniform(const str& name, GLfloat value);
		void SetUniform(const str& name, const glm::vec2& alue);
		void SetUniform(const str& name, const glm::vec3& alue);
		void SetUniform(const str& name, const glm::vec4& alue);

		//integers
		void SetUniform(const str& name, GLint value);
		void SetUniform(const str& name, const glm::ivec2& value);
		void SetUniform(const str& name, const glm::ivec3& value);
		void SetUniform(const str& name, const glm::ivec4& value);

		//matrices
		void SetUniform(const str& name, const glm::mat2& value);
		void SetUniform(const str& name, const glm::mat3& value);
		void SetUniform(const str& name, const glm::mat4& value);

		std::shared_ptr<UniformBufferInfo> GetUniformBlockInfo(const str& blockName) const;

	protected:
		GLuint LoadShader(GLenum _shaderType, const str& _text);

	private:
		GLuint m_programId;
		GLuint m_shaderId[5];
	};
}

#endif