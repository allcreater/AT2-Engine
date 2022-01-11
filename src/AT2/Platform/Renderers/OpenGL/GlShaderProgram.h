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
        NON_COPYABLE_OR_MOVABLE(GlShaderProgram)

        GlShaderProgram();
        ~GlShaderProgram() override;

    public:
        void Bind();
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
        std::shared_ptr<Introspection::ProgramInfo> m_uniformsInfo;

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
