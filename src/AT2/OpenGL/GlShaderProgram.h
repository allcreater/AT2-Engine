#ifndef AT2_GL_SHADERPROGRAM_H
#define AT2_GL_SHADERPROGRAM_H

#include "AT2lowlevel.h"
#include <map>


namespace AT2
{
	class GlShaderProgram : public IShaderProgram, public std::enable_shared_from_this<GlShaderProgram>
	{
	public:
		struct UniformInfo
		{
			GLint Index;
			GLint Offset;
			GLint ArrayStride, MatrixStride;
			GLint Type;

			UniformInfo(GLint index, GLint offset, GLint type, GLint arrayStride, GLint matrixStride) : Index(index), Offset(offset), ArrayStride(arrayStride), MatrixStride(matrixStride), Type(type) {}
			UniformInfo() : Index(0), Offset(0), ArrayStride(0), MatrixStride(0), Type(0){}
		};

		class UniformBufferInfo
		{
		friend class GlShaderProgram;

		public:
        [[nodiscard]] GLuint GetBlockIndex() const {return m_blockIndex;}
        [[nodiscard]] GLint GetBlockSize() const {return m_blockSize; }

        [[nodiscard]] const UniformInfo* GetUniformInfo(const str& name) const
			{
                const auto i = m_uniforms.find(name);
				if (i != m_uniforms.end())
					return &(i->second);
				
				return nullptr;
			}

		private:
			GLuint m_blockIndex {0};
			GLint m_blockSize {0};
			GLint m_numActiveUniforms {0};

			std::map<str, UniformInfo> m_uniforms;
		};

	public:
		NON_COPYABLE_OR_MOVABLE(GlShaderProgram)

		GlShaderProgram();
		~GlShaderProgram() override;

	public:
		void			Bind() override;
		unsigned int	GetId() const override { return m_programId; }
		bool			IsActive() const override;
        std::unique_ptr<IUniformContainer> CreateAssociatedUniformStorage(const str &blockName) override;

		void	AttachShader(const str& data, ShaderType type) override;

		//Warning: Shader reloading/relinking will invalidate that state
		void	SetUBO(const str& blockName, unsigned int index) override;
		void	SetUniform(const str& name, Uniform value) override;

		virtual const str& GetName() { return m_name; }
		virtual void SetName(const str& name) { m_name = name; }

	protected:
		std::shared_ptr<UniformBufferInfo> GetUniformBlockInfo(const str& blockName);
		bool TryCompile();
		void CleanUp();
		
	private:
		GLuint m_programId {0};
		std::vector<std::pair<ShaderType, GLuint>> m_shaderIds;
		std::unordered_map<str, std::shared_ptr<UniformBufferInfo>> m_uniformBlocksCache;
		str m_name;

        enum class State
        {
            Dirty,
            Ready,
            Error
        } m_currentState = State::Dirty;
	};
}

#endif