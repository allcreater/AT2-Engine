#ifndef AT2_GL_SHADERPROGRAM_H
#define AT2_GL_SHADERPROGRAM_H

#include <unordered_map>
#include "AT2lowlevel.h"
#include "GlProgramIntrospection.h"

namespace AT2::OpenGL
{
    //TODO: complete immutabilization
    class GlShaderProgram : public IShaderProgram, public std::enable_shared_from_this<GlShaderProgram>
    {
    public:
        
        enum class ShaderType : GLenum
        {
            Vertex = GL_VERTEX_SHADER,
        	TesselationControl = GL_TESS_CONTROL_SHADER,
            TesselationEvaluation = GL_TESS_EVALUATION_SHADER,
            Geometry = GL_GEOMETRY_SHADER,
            Fragment = GL_FRAGMENT_SHADER,
            Computational =  GL_COMPUTE_SHADER
        };
        using ShaderDescriptor = std::unordered_multimap<ShaderType, std::string>;

        GlShaderProgram(IRenderer& renderer, const ShaderDescriptor& descriptor);
        ~GlShaderProgram() override;

        GlShaderProgram(const GlShaderProgram&) = delete;
        GlShaderProgram& operator=(const GlShaderProgram&) = delete;
        GlShaderProgram(GlShaderProgram&& rhv) noexcept { swap(rhv); }
        GlShaderProgram& operator=(GlShaderProgram&& rhv) noexcept
        {
            swap(rhv);
            return *this;
        }

    public:
        std::unique_ptr<StructuredBuffer> CreateAssociatedUniformStorage(std::string_view blockName) override;

        //Warning: Shader reloading/relinking will invalidate that state
        void SetUBO(std::string_view blockName, unsigned int index);

        virtual const str& GetName() { return m_name; }
        virtual void SetName(const str& name) { m_name = name; }

        void swap(GlShaderProgram& rhv) noexcept
        {
            assert(m_renderer == rhv.m_renderer);

            std::swap(m_programId, rhv.m_programId);
            std::swap(m_shaderIds, rhv.m_shaderIds);
            std::swap(m_uniformsInfo, rhv.m_uniformsInfo);
            std::swap(m_name, rhv.m_name);
            std::swap(m_currentState, rhv.m_currentState);
        }

    //for internal usage
        void Bind();
        unsigned int GetId() const noexcept { return m_programId; }
        bool IsActive() const noexcept;

        void SetUniform(std::string_view name, Uniform value);
        void SetUniformArray(std::string_view name, UniformArray value);
        std::optional<unsigned int> GetUniformBufferLocation(std::string_view name);

    protected:
        bool TryLinkProgram();
        
    private:
        IRenderer* m_renderer;
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
