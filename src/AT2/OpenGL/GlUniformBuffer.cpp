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
void SetUniformInternal(GLuint bufferId, GlShaderProgram::UniformBufferInfo* ubi, const str& name, const T& value)
{
    const auto* ui = ubi->GetUniformInfo(name);
    if (!ui)
        return;

    auto* const data = static_cast<GLbyte*>(glMapNamedBufferRangeEXT(bufferId, ui->Offset, sizeof(T), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT));
    const void* ptr = value_ptr(value);
    memcpy(data, ptr, sizeof(T));
    glUnmapNamedBufferEXT(bufferId);
}


template <typename T, size_t C, size_t R>
void SetUniformInternal(GLuint bufferId, GlShaderProgram::UniformBufferInfo* ubi, const str& name, const glm::mat<C, R, T>& value)
{
    using MatT = mat<C, R, T>;//decltype(value);
    
    const auto* ui = ubi->GetUniformInfo(name);
    if (!ui)
        return;

    auto* const data = static_cast<GLbyte*>(glMapNamedBufferRangeEXT(bufferId, ui->Offset, ui->MatrixStride * C, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT));
    for (int i = 0; i < C; ++i)
        memcpy(data + ui->MatrixStride * i, &(value[i].x), sizeof(MatT::col_type));

    glUnmapNamedBufferEXT(bufferId);
}


void GlUniformBuffer::SetUniform(const str &name, const Uniform &value)
{
    using namespace glm;

    std::visit([&](const auto& x) { SetUniformInternal(m_id, m_ubi.get(), name, x); }, value);
}

void GlUniformBuffer::Bind(IStateManager &stateManager) const
{
    glBindBufferBase(GL_UNIFORM_BUFFER, m_bindingPoint, m_id);
}


void GlUniformBuffer::SetUniform(const str& name, std::weak_ptr<const ITexture> value) { throw std::logic_error("not implemented"); }