#include "GlShaderProgram.h"

#include "../UniformContainer.h"
#include "GlUniformBuffer.h"
#include "Mappings.h"

using namespace AT2;
using namespace AT2::OpenGl::Introspection;

GLuint LoadShader(GLenum _shaderType, const str& _text)
{
    const GLuint shader = glCreateShader(_shaderType);

    const GLchar* data = _text.c_str();
    auto dataLength = static_cast<GLint>(_text.length());
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

GlShaderProgram::GlShaderProgram()
{
    m_programId = glCreateProgram();
    if (m_programId == invalid_index)
        throw AT2Exception(AT2Exception::ErrorCase::Shader, "couldn't acquire program handle");
}

GlShaderProgram::~GlShaderProgram()
{
    CleanUp();
    if (m_programId != invalid_index)
        glDeleteProgram(m_programId);
}

void GlShaderProgram::AttachShader(const str& _code, ShaderType _type)
{
    if (_code.empty())
        throw AT2Exception(AT2Exception::ErrorCase::Shader, "GlShaderProgram: trying to attach empty shader");

    const GLuint shaderId = LoadShader(Mappings::TranslateShaderType(_type), _code);
    glAttachShader(m_programId, shaderId);

    m_shaderIds.emplace_back(_type, shaderId);

    m_currentState = State::Dirty;
}

const std::shared_ptr<ProgramInfo>& GlShaderProgram::GetIntrospection()
{
    if (TryCompile())
    {
        if (!m_uniformsInfo)
            m_uniformsInfo = ProgramInfo::Request(m_programId);
    }
    else
    {
        assert(!m_uniformsInfo);
        m_uniformsInfo.reset();
    }

    return m_uniformsInfo;

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
            std::string infoLog(infoLogLength, '\0');

            glGetShaderInfoLog(shaderId, static_cast<GLsizei>(infoLog.size()), &infoLogLength, infoLog.data());
            if (infoLogLength > 0)
                Log::Debug() << "Shader \"" << GetName() << "\" log: " << std::endl << infoLog;

            GLint status = 0;
            glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);
            if (status != GL_TRUE)
            {
                Log::Warning() << ShaderTypeName(shaderType) << " shader \"" << GetName() << "\" compilation failed!"
                               << std::endl
                               << infoLog;
            }
        }

        //linking program
        glLinkProgram(m_programId);
        GLint isLinked = 0;
        glGetProgramiv(m_programId, GL_LINK_STATUS, &isLinked);

        //log
        GLint infoLogLength = 0;
        glGetProgramiv(m_programId, GL_INFO_LOG_LENGTH, &infoLogLength);
        std::string infoLog(infoLogLength, '\0');

        glGetProgramInfoLog(m_programId, 2048, &infoLogLength, infoLog.data());
        if (infoLogLength > 0)
            Log::Debug() << "Shader program log: " << std::endl << infoLog;

        m_currentState = (isLinked) ? State::Ready : State::Error;
    }

    return m_currentState == State::Ready;
}

std::unique_ptr<IUniformContainer> GlShaderProgram::CreateAssociatedUniformStorage(std::string_view blockName)
{
    if (blockName.empty())
        return std::make_unique<AT2::UniformContainer>();

    if (const auto& programInfo = GetIntrospection())
    {
        const auto uniformBlockInfo = programInfo->getUniformBlock(blockName);
        if (!uniformBlockInfo)
            return nullptr;

        auto uniformBuffer =
            std::make_unique<GlUniformBuffer>(std::shared_ptr<const UniformBlockInfo>(programInfo, uniformBlockInfo));

        //just initial binding to make buffer usable "out of box". External code could rebind it or remap as user wants.
        uniformBuffer->SetBindingPoint(uniformBlockInfo->InitialBinding);

        return uniformBuffer;
    }

    return nullptr;
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

void GlShaderProgram::SetUBO(const str& blockName, unsigned int index)
{
    const GLuint blockIndex = glGetUniformBlockIndex(m_programId, blockName.c_str());
    if (blockIndex != GL_INVALID_INDEX)
        glUniformBlockBinding(m_programId, blockIndex, index);
    else
        Log::Warning() << "Uniform block " << blockName << "not found" << std::endl;
}

void GlShaderProgram::SetUniform(const str& name, Uniform value)
{
    if (m_currentState != State::Ready)
        return;

    using namespace glm;

    const auto location = glGetUniformLocation(m_programId, name.c_str());

    std::visit(Utils::overloaded {
                   [&](int x) { glProgramUniform1i(m_programId, location, x); },
                   [&](const ivec2& x) { glProgramUniform2iv(m_programId, location, 1, value_ptr(x)); },
                   [&](const ivec3& x) { glProgramUniform3iv(m_programId, location, 1, value_ptr(x)); },
                   [&](const ivec4& x) { glProgramUniform4iv(m_programId, location, 1, value_ptr(x)); },

                   [&](unsigned int x) { glProgramUniform1ui(m_programId, location, x); },
                   [&](const uvec2& x) { glProgramUniform2uiv(m_programId, location, 1, value_ptr(x)); },
                   [&](const uvec3& x) { glProgramUniform3uiv(m_programId, location, 1, value_ptr(x)); },
                   [&](const uvec4& x) { glProgramUniform4uiv(m_programId, location, 1, value_ptr(x)); },

                   [&](float x) { glProgramUniform1f(m_programId, location, x); },
                   [&](const vec2& x) { glProgramUniform2fv(m_programId, location, 1, value_ptr(x)); },
                   [&](const vec3& x) { glProgramUniform3fv(m_programId, location, 1, value_ptr(x)); },
                   [&](const vec4& x) { glProgramUniform4fv(m_programId, location, 1, value_ptr(x)); },
                   [&](const mat2& x) { glProgramUniformMatrix2fv(m_programId, location, 1, false, value_ptr(x)); },
                   [&](const mat3& x) { glProgramUniformMatrix3fv(m_programId, location, 1, false, value_ptr(x)); },
                   [&](const mat4& x) { glProgramUniformMatrix4fv(m_programId, location, 1, false, value_ptr(x)); },

                   [&](double x) { glProgramUniform1d(m_programId, location, x); },
                   [&](const dvec2& x) { glProgramUniform2dv(m_programId, location, 1, value_ptr(x)); },
                   [&](const dvec3& x) { glProgramUniform3dv(m_programId, location, 1, value_ptr(x)); },
                   [&](const dvec4& x) { glProgramUniform4dv(m_programId, location, 1, value_ptr(x)); },
                   [&](const dmat2& x) { glProgramUniformMatrix2dv(m_programId, location, 1, false, value_ptr(x)); },
                   [&](const dmat3& x) { glProgramUniformMatrix3dv(m_programId, location, 1, false, value_ptr(x)); },
                   [&](const dmat4& x) { glProgramUniformMatrix4dv(m_programId, location, 1, false, value_ptr(x)); }},
               value);
}

