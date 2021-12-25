#ifndef AT2_GL_SHADERPROGRAM_H
#define AT2_GL_SHADERPROGRAM_H

#include <map>
#include "AT2lowlevel.h"
#include "GlProgramIntrospection.h"

namespace AT2::OpenGL
{
    class GlShaderProgram : public IShaderProgram, public std::enable_shared_from_this<GlShaderProgram>
    {
    public:
        //struct UniformInfo
        //{
        //    GLint Index;
        //    GLint Offset;
        //    GLint ArrayStride, MatrixStride;
        //    GLint Type;

        //    UniformInfo(GLint index, GLint offset, GLint type, GLint arrayStride, GLint matrixStride) :
        //        Index(index), Offset(offset), ArrayStride(arrayStride), MatrixStride(matrixStride), Type(type)
        //    {
        //    }
        //    UniformInfo() : Index(0), Offset(0), ArrayStride(0), MatrixStride(0), Type(0) {}
        //};

        //class UniformBufferInfo
        //{
        //    friend class GlShaderProgram;

        //public:
        //    [[nodiscard]] GLuint GetBlockIndex() const { return m_blockIndex; }
        //    [[nodiscard]] GLint GetBlockSize() const { return m_blockSize; }

        //    [[nodiscard]] const UniformInfo* GetUniformInfo(const str& name) const
        //    {
        //        const auto i = m_uniforms.find(name);
        //        if (i != m_uniforms.end())
        //            return &(i->second);

        //        return nullptr;
        //    }

        //private:
        //    GLuint m_blockIndex {0};
        //    GLint m_blockSize {0};
        //    GLint m_numActiveUniforms {0};

        //    std::map<str, UniformInfo> m_uniforms;
        //};

    public:
        NON_COPYABLE_OR_MOVABLE(GlShaderProgram)

        GlShaderProgram();
        ~GlShaderProgram() override;

    public:
        void Bind() override;
        unsigned int GetId() const noexcept override { return m_programId; }
        bool IsActive() const noexcept override;
        std::unique_ptr<IUniformContainer> CreateAssociatedUniformStorage(std::string_view blockName) override;

        void AttachShader(std::string_view data, ShaderType type) override;

        //Warning: Shader reloading/relinking will invalidate that state
        void SetUBO(std::string_view blockName, unsigned int index) override;
        void SetUniform(std::string_view name, Uniform value) override;
        void SetUniformArray(std::string_view name, UniformArray value) override;

        virtual const str& GetName() { return m_name; }
        virtual void SetName(const str& name) { m_name = name; }

    protected:
        bool TryCompile();
        void CleanUp();

    private:
        GLuint m_programId {0};
        std::vector<std::pair<ShaderType, GLuint>> m_shaderIds;
        std::shared_ptr<OpenGL::Introspection::ProgramInfo> m_uniformsInfo;

        str m_name;

        enum class State
        {
            Dirty,
            Ready,
            Error
        } m_currentState = State::Dirty;
    };
} // namespace AT2

#endif