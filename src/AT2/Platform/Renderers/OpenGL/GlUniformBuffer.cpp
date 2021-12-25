#include "GlUniformBuffer.h"
#include <BufferMapperGuard.h>

#include <utility>

using namespace AT2;
using namespace OpenGL;
using namespace Introspection;
using namespace glm;

GlUniformBuffer::GlUniformBuffer(std::shared_ptr<const UniformBlockInfo> ubi) :
    GlVertexBuffer(VertexBufferType::UniformBuffer), m_uniformBlockInfo(std::move(ubi)), m_bindingPoint(0)
{
    constexpr std::byte* emptyData = nullptr;
    SetDataRaw(std::span{emptyData, m_uniformBlockInfo->DataSize});
}

namespace
{
    template <typename T>
    const GLvoid* value_ptr(const T& value)
    {
        return static_cast<const GLvoid*>(&value);
    }


    template <typename T>
    void SetUniformInternal(GlUniformBuffer& buffer, const UniformBlockInfo& ubi, std::string_view name, const T& value)
    {
        const auto* ui = AT2::Utils::find(ubi.Uniforms, name);
        if (!ui)
            return;

        //Unfortunately high-level methods are slowly than direct API calls :( But it could be inlined or something like that if we need it.
        BufferMapperGuard mapping {buffer, static_cast<size_t>(ui->Offset), sizeof(T), BufferUsage::Write};
        mapping.Set(value);
    }


    template <typename T, length_t C, length_t R, qualifier Q>
    void SetUniformInternal(GlUniformBuffer& buffer, const UniformBlockInfo& ubi, std::string_view name, const glm::mat<C, R, T, Q>& value)
    {
        const auto* ui = AT2::Utils::find(ubi.Uniforms, name);
        if (!ui)
            return;

        BufferMapperGuard mapping {buffer, static_cast<size_t>(ui->Offset), static_cast<size_t>(C) * ui->MatrixStride, BufferUsage::Write};
        for (decltype(C) i = 0; i < C; ++i)
            mapping.Set(value[i], static_cast<size_t>(i) * ui->MatrixStride);
    }
} // namespace

void GlUniformBuffer::SetUniform(std::string_view name, const Uniform& value)
{
    using namespace glm;

    std::visit([&](const auto& x) { SetUniformInternal(*this, *m_uniformBlockInfo, name, x); }, value);
}

void GlUniformBuffer::Bind(IStateManager&) const
{
    glBindBufferBase(GL_UNIFORM_BUFFER, m_bindingPoint, GetId());
}


void GlUniformBuffer::SetUniform(std::string_view, const std::shared_ptr<ITexture>&)
{
    throw std::logic_error("not implemented");
}