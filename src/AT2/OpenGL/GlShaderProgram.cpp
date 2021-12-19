#include "GlShaderProgram.h"

#include "../UniformContainer.h"
#include "GlUniformBuffer.h"
#include "Mappings.h"

using namespace AT2;
using namespace OpenGL;
using namespace Introspection;

namespace
{
    template <typename T>
    auto span_value_ptr(const std::span<T>& value)
    {
        if constexpr (requires { typename T::value_type; })
        {
            return glm::value_ptr(value[0]);
        }
        else
        {
            return value.data();
        }
    }

    GLuint LoadShader(GLenum _shaderType, std::string_view source)
    {
        const GLuint shader = glCreateShader(_shaderType);

        const GLchar* data = source.data();
        auto dataLength = static_cast<GLint>(source.length());
        glShaderSource(shader, 1, &data, &dataLength);

        glCompileShader(shader);

        return shader;
    }

    constexpr std::string_view ShaderTypeName(ShaderType type)
    {
        using namespace std::literals;
        switch (type)
        {
        case ShaderType::Vertex: return "Vertex"sv;
        case ShaderType::TesselationControl: return "Tesselation control"sv;
        case ShaderType::TesselationEvaluation: return "Tesselation evaluation"sv;
        case ShaderType::Geometry: return "Geometry shader"sv;
        case ShaderType::Fragment: return "Fragmens shader"sv;
        case ShaderType::Computational: return "Compute shader"sv;
        default: return ""sv;
        }
    }

    std::optional<int> GetUniformLocation(ProgramInfo& programInfo, std::string_view name) 
    {
        auto uniformsIt = programInfo.getUniforms().find(name);
        if (uniformsIt == programInfo.getUniforms().end())
            return std::nullopt;

        return uniformsIt->second.Location;
    }


    using namespace glm;
    void SetProgramUniformArray(GLuint programId, GLint location, std::span<const int> values)   { glProgramUniform1iv(programId, location, static_cast<int>(values.size()), span_value_ptr(values)); }
    void SetProgramUniformArray(GLuint programId, GLint location, std::span<const ivec2> values) { glProgramUniform2iv(programId, location, static_cast<int>(values.size()), span_value_ptr(values)); }
    void SetProgramUniformArray(GLuint programId, GLint location, std::span<const ivec3> values) { glProgramUniform3iv(programId, location, static_cast<int>(values.size()), span_value_ptr(values)); }
    void SetProgramUniformArray(GLuint programId, GLint location, std::span<const ivec4> values) { glProgramUniform4iv(programId, location, static_cast<int>(values.size()), span_value_ptr(values)); }

    void SetProgramUniformArray(GLuint programId, GLint location, std::span<const uint> values)  { glProgramUniform1uiv(programId, location, static_cast<int>(values.size()), span_value_ptr(values)); }
    void SetProgramUniformArray(GLuint programId, GLint location, std::span<const uvec2> values) { glProgramUniform2uiv(programId, location, static_cast<int>(values.size()), span_value_ptr(values)); }
    void SetProgramUniformArray(GLuint programId, GLint location, std::span<const uvec3> values) { glProgramUniform3uiv(programId, location, static_cast<int>(values.size()), span_value_ptr(values)); }
    void SetProgramUniformArray(GLuint programId, GLint location, std::span<const uvec4> values) { glProgramUniform4uiv(programId, location, static_cast<int>(values.size()), span_value_ptr(values)); }

    void SetProgramUniformArray(GLuint programId, GLint location, std::span<const float> values) { glProgramUniform1fv(programId, location, static_cast<int>(values.size()), span_value_ptr(values)); }
    void SetProgramUniformArray(GLuint programId, GLint location, std::span<const vec2> values)  { glProgramUniform2fv(programId, location, static_cast<int>(values.size()), span_value_ptr(values)); }
    void SetProgramUniformArray(GLuint programId, GLint location, std::span<const vec3> values)  { glProgramUniform3fv(programId, location, static_cast<int>(values.size()), span_value_ptr(values)); }
    void SetProgramUniformArray(GLuint programId, GLint location, std::span<const vec4> values)  { glProgramUniform4fv(programId, location, static_cast<int>(values.size()), span_value_ptr(values)); }
    void SetProgramUniformArray(GLuint programId, GLint location, std::span<const mat2> values)  { glProgramUniformMatrix2fv(programId, location, static_cast<int>(values.size()), false, span_value_ptr(values)); }
    void SetProgramUniformArray(GLuint programId, GLint location, std::span<const mat3> values)  { glProgramUniformMatrix3fv(programId, location, static_cast<int>(values.size()), false, span_value_ptr(values)); }
    void SetProgramUniformArray(GLuint programId, GLint location, std::span<const mat4> values)  { glProgramUniformMatrix4fv(programId, location, static_cast<int>(values.size()), false, span_value_ptr(values)); }

    void SetProgramUniformArray(GLuint programId, GLint location, std::span<const double> values){ glProgramUniform1dv(programId, location, static_cast<int>(values.size()), span_value_ptr(values)); }
    void SetProgramUniformArray(GLuint programId, GLint location, std::span<const dvec2> values) { glProgramUniform2dv(programId, location, static_cast<int>(values.size()), span_value_ptr(values)); }
    void SetProgramUniformArray(GLuint programId, GLint location, std::span<const dvec3> values) { glProgramUniform3dv(programId, location, static_cast<int>(values.size()), span_value_ptr(values)); }
    void SetProgramUniformArray(GLuint programId, GLint location, std::span<const dvec4> values) { glProgramUniform4dv(programId, location, static_cast<int>(values.size()), span_value_ptr(values)); }
    void SetProgramUniformArray(GLuint programId, GLint location, std::span<const dmat2> values) { glProgramUniformMatrix2dv(programId, location, static_cast<int>(values.size()), false, span_value_ptr(values)); }
    void SetProgramUniformArray(GLuint programId, GLint location, std::span<const dmat3> values) { glProgramUniformMatrix3dv(programId, location, static_cast<int>(values.size()), false, span_value_ptr(values)); }
    void SetProgramUniformArray(GLuint programId, GLint location, std::span<const dmat4> values) { glProgramUniformMatrix4dv(programId, location, static_cast<int>(values.size()), false, span_value_ptr(values)); }

} // namespace


GlShaderProgram::GlShaderProgram()
{
    m_programId = glCreateProgram();
    assert(m_programId);
}

GlShaderProgram::~GlShaderProgram()
{
    CleanUp();
    glDeleteProgram(m_programId);
}

void GlShaderProgram::AttachShader(std::string_view _code, ShaderType _type)
{
    if (_code.empty())
        throw AT2ShaderException( "GlShaderProgram: trying to attach empty shader");

    const GLuint shaderId = LoadShader(Mappings::TranslateShaderType(_type), _code);
    glAttachShader(m_programId, shaderId);

    m_shaderIds.emplace_back(_type, shaderId);

    m_currentState = State::Dirty;
}

bool GlShaderProgram::TryCompile()
{
    if (m_currentState == State::Dirty)
    {
        m_uniformsInfo.reset();

        for (const auto& [shaderType, shaderId] : m_shaderIds)
        {
            GLint infoLogLength = 0;
            glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength);
            //if (infoLogLength > 0)
            //{
            //    std::string infoLog(static_cast<unsigned>(infoLogLength) - 1, '\0');

            //    glGetShaderInfoLog(shaderId, infoLogLength, &infoLogLength, infoLog.data());
            //    Log::Debug() << "Shader \"" << GetName() << "\" log: " << std::endl << infoLog;
            //}

            GLint status = 0;
            glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);
            if (status != GL_TRUE)
            {
                Log::Warning() << ShaderTypeName(shaderType) << " shader \"" << GetName() << "\" compilation failed!" << std::endl;
            }
        }

        //linking program
        glLinkProgram(m_programId);
        GLint isLinked = 0;
        glGetProgramiv(m_programId, GL_LINK_STATUS, &isLinked);

        if (isLinked)
        {
            m_uniformsInfo = ProgramInfo::Request(m_programId);
            m_currentState = State::Ready;
        }
        else
            m_currentState = State::Error;

        //log
        GLint infoLogLength = 0;
        glGetProgramiv(m_programId, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            std::string infoLog(static_cast<unsigned>(infoLogLength) - 1, '\0');

            glGetProgramInfoLog(m_programId, infoLogLength, &infoLogLength, infoLog.data());
            Log::Debug() << "Shader program log: " << std::endl << infoLog;
        }
    }

    return m_currentState == State::Ready;
}

std::unique_ptr<IUniformContainer> GlShaderProgram::CreateAssociatedUniformStorage(std::string_view blockName)
{
    if (blockName.empty())
        return std::make_unique<AT2::UniformContainer>();

    if (!TryCompile())
        return nullptr;
    assert(m_uniformsInfo);

    const auto uniformBlockInfo = m_uniformsInfo->getUniformBlock(blockName);
    if (!uniformBlockInfo)
        return nullptr;

    auto uniformBuffer = std::make_unique<GlUniformBuffer>(std::shared_ptr<const UniformBlockInfo>(m_uniformsInfo, uniformBlockInfo));

    //just initial binding to make buffer usable "out of box". External code could rebind it or remap as user wants.
    uniformBuffer->SetBindingPoint(uniformBlockInfo->InitialBinding);

    return uniformBuffer;
}

void GlShaderProgram::CleanUp()
{
    for (const auto& [type, shaderId] : m_shaderIds)
    {
        glDetachShader(m_programId, shaderId);
        glDeleteShader(shaderId);
    }

    m_shaderIds.clear();
    m_uniformsInfo.reset();

    m_currentState = State::Dirty;
}

void GlShaderProgram::Bind()
{
    if (TryCompile())
        glUseProgram(m_programId);
}

bool GlShaderProgram::IsActive() const noexcept
{
    GLint activeProgramId = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &activeProgramId);

    return activeProgramId > 0 && m_programId == static_cast<GLuint>(activeProgramId);
}

void GlShaderProgram::SetUBO(std::string_view blockName, unsigned int index)
{
    if (!TryCompile())
        return;
    assert(m_uniformsInfo);


    const GLuint blockIndex = m_uniformsInfo->getUniformBlock(blockName)->BlockIndex;
    if (blockIndex != GL_INVALID_INDEX)
        glUniformBlockBinding(m_programId, blockIndex, index);
    else
        Log::Warning() << "Uniform block " << blockName << "not found" << std::endl;
}

void GlShaderProgram::SetUniform(std::string_view name, Uniform value)
{
    if (!TryCompile())
        return;
    assert(m_uniformsInfo);

    if (const auto location = GetUniformLocation(*m_uniformsInfo, name))
        std::visit([&]<typename T>(const T& val) { SetProgramUniformArray(m_programId, *location, std::span<const T> {&val, 1}); },
                   value);
}

void GlShaderProgram::SetUniformArray(std::string_view name, UniformArray value)
{
    if (!TryCompile())
        return;
    assert(m_uniformsInfo);

    if (const auto location = GetUniformLocation(*m_uniformsInfo, name))
        std::visit([&](const auto& valueSpan) { SetProgramUniformArray(m_programId, *location, valueSpan); }, value);
}