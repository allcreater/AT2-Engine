#ifndef AT2_GL_SHADERPROGRAM_H
#define AT2_GL_SHADERPROGRAM_H

#include "AT2lowlevel.h"
#include <map>


namespace AT2
{
	enum class GlShaderType
	{
		//Computational = GL_COMPUTE_SHADER,
		Vertex = 0,
		TesselationControl = 1,
		TesselationEvaluation = 2,
		Geometry = 3,
		Fragment = 4
	};

	class GlShaderProgram : public IShaderProgram, public std::enable_shared_from_this<GlShaderProgram>
	{
	public:
		struct UniformInfo
		{
			GLint Index;
			GLint Offset;
			GLint ArrayStride, MatrixStride;
			GLint Type;

			UniformInfo(GLint index, GLint offset, GLint type, GLint arrayStride, GLint matrixStride) : Index(index), Offset(offset), Type(type), ArrayStride(arrayStride), MatrixStride(matrixStride) {}
			UniformInfo() : Index(0), Offset(0), Type(0), ArrayStride(0), MatrixStride(0){}
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
		~GlShaderProgram();

	public:
		void			Bind() override;
		unsigned int	GetId() const override { return m_programId; }
		bool			IsActive() const override;
		bool			Compile()  override;
		std::shared_ptr<IUniformContainer> CreateAssociatedUniformStorage() override;

		virtual void	AttachShader(const str& code, GlShaderType type);

		//Warning: Shader reloading/relinking will invalidate that state
		virtual void	SetUBO(const str& blockName, unsigned int index);

		virtual const str& GetName() { return m_name; }
		virtual void SetName(const str& name) { m_name = name; }

		std::shared_ptr<UniformBufferInfo> GetUniformBlockInfo(const str& blockName) const;

	protected:
		GLuint LoadShader(GLenum _shaderType, const str& _text);
		void CleanUp();
		
	private:
		GLuint m_programId;
		std::vector<GLuint> m_shaderId[5];
		str m_name;
	};
}

#endif