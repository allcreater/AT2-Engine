#ifndef AT2_GL_SHADERPROGRAM_H
#define AT2_GL_SHADERPROGRAM_H

#include <map>
#include "AT2lowlevel.h"
#include "GlProgramIntrospection.h"

namespace AT2
{
    class GlShaderProgram : public IShaderProgram, private std::enable_shared_from_this<GlShaderProgram>
    {
    public:
        NON_COPYABLE_OR_MOVABLE(GlShaderProgram)

        GlShaderProgram();
        ~GlShaderProgram() override;

    public:
        void Bind() override;
        unsigned int GetId() const noexcept override { return m_programId; }
        bool IsActive() const noexcept override;
        std::unique_ptr<IUniformContainer> CreateAssociatedUniformStorage(std::string_view blockName) override;

        void AttachShader(const str& data, ShaderType type) override;

        //Warning: Shader reloading/relinking will invalidate that state
        void SetUBO(const str& blockName, unsigned int index) override;
        void SetUniform(const str& name, Uniform value) override;

        virtual const str& GetName() { return m_name; }
        virtual void SetName(const str& name) { m_name = name; }

    protected:
        const std::shared_ptr<OpenGl::Introspection::ProgramInfo>& GetIntrospection();
        bool TryCompile();
        void CleanUp();

        constexpr static GLuint invalid_index = 0;

    private:
        GLuint m_programId = invalid_index;
        std::vector<std::pair<ShaderType, GLuint>> m_shaderIds;
        std::shared_ptr<OpenGl::Introspection::ProgramInfo> m_uniformsInfo;

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