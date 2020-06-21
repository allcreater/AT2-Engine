#include "GlUniformBuffer.h"

using namespace AT2;

using namespace glm;

GlUniformBuffer::GlUniformBuffer(std::shared_ptr<GlShaderProgram::UniformBufferInfo> ubi) : 
    GlVertexBuffer (VertexBufferType::UniformBuffer),
    m_bindingPoint(0),
    m_ubi(ubi)
{
    m_length = m_ubi->GetBlockSize();

    glNamedBufferDataEXT(m_id, m_length, 0, static_cast<GLenum>(m_usageHint));
}

GlUniformBuffer::~GlUniformBuffer()
{
    glDeleteBuffers(1, &m_id);
}

template <typename T>
const GLvoid* value_ptr(const T& value) { return reinterpret_cast<const GLvoid*>(&value); }


template <typename T>
void SetUniformInternal(GLuint bufferId, GlShaderProgram::UniformBufferInfo* ubi, const str& name, const T& value)
{
    const auto* ui = ubi->GetUniformInfo(name);
    if (!ui)
        return;

    //glMapNamedBufferRangeEXT(bufferId, ui->Offset, size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
    auto data = reinterpret_cast<GLbyte*>(glMapNamedBufferEXT(bufferId, GL_WRITE_ONLY));
    const void* ptr = value_ptr(value);
    memcpy(data + ui->Offset, ptr, sizeof(T));
    glUnmapNamedBufferEXT(bufferId);
}


template <typename T, size_t C, size_t R>
void SetUniformInternal(GLuint bufferId, GlShaderProgram::UniformBufferInfo* ubi, const str& name, const glm::mat<C, R, T>& value)
{
    using MatT = mat<C, R, T>;//decltype(value);
    
    const auto* ui = ubi->GetUniformInfo(name);
    if (!ui)
        return;

    //glMapNamedBufferRangeEXT(bufferId, ui->Offset, size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
    auto data = reinterpret_cast<GLbyte*>(glMapNamedBufferEXT(bufferId, GL_WRITE_ONLY));
    for (int i = 0; i < C; ++i)
        memcpy(data + ui->Offset + ui->MatrixStride * i, &(value[i].x), sizeof(MatT::col_type));

    glUnmapNamedBufferEXT(bufferId);
}


void GlUniformBuffer::SetUniform(const str &name, const Uniform &value)
{
    using namespace glm;

    //could be reduced to just one generic lambda, but let it be
    std::visit(Utils::overloaded{
        [&](int x) { SetUniformInternal(m_id, m_ubi.get(), name, x); },
        [&](const ivec2& x) { SetUniformInternal(m_id, m_ubi.get(), name, x); },
        [&](const ivec3& x) { SetUniformInternal(m_id, m_ubi.get(), name, x); },
        [&](const ivec4& x) { SetUniformInternal(m_id, m_ubi.get(), name, x); },

        [&](unsigned int x) { SetUniformInternal(m_id, m_ubi.get(), name, x); },
        [&](const uvec2& x) { SetUniformInternal(m_id, m_ubi.get(), name, x); },
        [&](const uvec3& x) { SetUniformInternal(m_id, m_ubi.get(), name, x); },
        [&](const uvec4& x) { SetUniformInternal(m_id, m_ubi.get(), name, x); },

        [&](float x)       { SetUniformInternal(m_id, m_ubi.get(), name, x); },
        [&](const vec2& x) { SetUniformInternal(m_id, m_ubi.get(), name, x); },
        [&](const vec3& x) { SetUniformInternal(m_id, m_ubi.get(), name, x); },
        [&](const vec4& x) { SetUniformInternal(m_id, m_ubi.get(), name, x); },
        [&](const mat2& x) { SetUniformInternal(m_id, m_ubi.get(), name, x); },
        [&](const mat3& x) { SetUniformInternal(m_id, m_ubi.get(), name, x); },
        [&](const mat4& x) { SetUniformInternal(m_id, m_ubi.get(), name, x); },

        [&](double x)       { SetUniformInternal(m_id, m_ubi.get(), name, x); },
        [&](const dvec2& x) { SetUniformInternal(m_id, m_ubi.get(), name, x); },
        [&](const dvec3& x) { SetUniformInternal(m_id, m_ubi.get(), name, x); },
        [&](const dvec4& x) { SetUniformInternal(m_id, m_ubi.get(), name, x); },
        [&](const dmat2& x) { SetUniformInternal(m_id, m_ubi.get(), name, x); },
        [&](const dmat3& x) { SetUniformInternal(m_id, m_ubi.get(), name, x); },
        [&](const dmat4& x) { SetUniformInternal(m_id, m_ubi.get(), name, x); }
        }, value);
}

void GlUniformBuffer::Bind()
{
    glBindBufferBase(GL_UNIFORM_BUFFER, m_bindingPoint, m_id);
}


void GlUniformBuffer::SetUniform(const str& name, std::weak_ptr<const ITexture> value) { throw std::logic_error("not implemented"); }