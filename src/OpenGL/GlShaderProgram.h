#ifndef AT2_GL_SHADERPROGRAM_H
#define AT2_GL_SHADERPROGRAM_H

#include "AT2lowlevel.h"
#include <map>

namespace AT2
{
	class GlUniformBuffer;

	class GlShaderProgram : public IShaderProgram
	{
	public:
		struct UniformInfo
		{
			GLint Index;
			GLint Offset;
			GLint Type;

			UniformInfo(GLint index, GLint offset, GLint type) : Index(index), Offset(offset), Type(type) {}
			UniformInfo() : Index(0), Offset(0), Type(0) {}
		};

		class UniformBufferInfo
		{
		friend class GlShaderProgram;

		public:
			GLuint	GetBlockIndex() const {return m_blockIndex;}
			GLint	GetBlockSize() const {return m_blockSize; }

			const UniformInfo* GetUniformInfo(const str& name) const
			{
				auto i = m_uniforms.find(name);
				if (i != m_uniforms.end())
					return &(i->second);
				
				return nullptr;
			}
		private:
			GLuint m_blockIndex; 
			GLint m_blockSize;
			GLint m_numActiveUniforms;

			std::map<str, UniformInfo> m_uniforms;
		};

	public:
		GlShaderProgram();
		GlShaderProgram(const str& vs, const str& tcs, const str& tes, const str& gs, const str& fs);
		virtual ~GlShaderProgram();

		virtual void Bind();
		virtual unsigned int GetId() const { return m_programId; }

		virtual void Reload(const str& vs, const str& tcs, const str& tes, const str& gs, const str& fs); //TODO maybe we need to make it more flexible

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

		void SetUBO(const str& blockName, unsigned int index);

		std::shared_ptr<UniformBufferInfo> GetUniformBlockInfo(const str& blockName) const;
	protected:
		GLuint LoadShader(GLenum _shaderType, const str& _text);
		void CleanUp();

	private:
		GLuint m_programId;
		GLuint m_shaderId[5];
	};
}

#endif