#include "GlUniformBuffer.h"

#include <utility>

using namespace AT2;

using namespace glm;

GlUniformBuffer::GlUniformBuffer(std::shared_ptr<GlShaderProgram::UniformBufferInfo> ubi) : 
    GlVertexBuffer (VertexBufferType::UniformBuffer),
    m_ubi(std::move(ubi)),
    m_bindingPoint(0)
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
void SetUniformInternal(GLuint bufferId, const GlShaderProgram::UniformBufferInfo& ubi, const str& name, const T& value)
{
    const auto* ui = ubi.GetUniformInfo(name);
    if (!ui)
        return;

    auto* const data = static_cast<std::byte*>(glMapNamedBufferRangeEXT(bufferId, ui->Offset, sizeof(T), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT));
    const void* ptr = value_ptr(value);
    memcpy(data, ptr, sizeof(T));
    glUnmapNamedBufferEXT(bufferId);
}


template <typename T, length_t C, length_t R, qualifier Q>
void SetUniformInternal(GLuint bufferId, const GlShaderProgram::UniformBufferInfo& ubi, const str& name, const glm::mat<C, R, T, Q>& value)
{
    using MatT = mat<C, R, T, Q>;
    
    const auto* ui = ubi.GetUniformInfo(name);
    if (!ui)
        return;

    auto* const data = static_cast<std::byte*>(glMapNamedBufferRangeEXT(bufferId, ui->Offset, static_cast<GLsizeiptr>(C) * ui->MatrixStride, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT));
    for (decltype(C) i = 0; i < C; ++i)
        memcpy(data + static_cast<size_t>(i) * ui->MatrixStride, value_ptr(value[i]), sizeof(typename MatT::col_type));

    glUnmapNamedBufferEXT(bufferId);
}


void GlUniformBuffer::SetUniform(const str &name, const Uniform &value)
{
    using namespace glm;

    std::visit([&](const auto& x) { SetUniformInternal(m_id, *m_ubi, name, x); }, value);
}

void GlUniformBuffer::Bind(IStateManager &stateManager) const
{
    glBindBufferBase(GL_UNIFORM_BUFFER, m_bindingPoint, m_id);
}


void GlUniformBuffer::SetUniform(const str& name, const std::shared_ptr<ITexture> &value) { throw std::logic_error("not implemented"); }