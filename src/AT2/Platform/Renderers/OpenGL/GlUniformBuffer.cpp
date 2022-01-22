#include "GlUniformBuffer.h"
#include <DataLayout/IO.hpp>

#include <utility>

using namespace AT2;
using namespace OpenGL;
using namespace Introspection;
using namespace glm;

GlUniformBuffer::GlUniformBuffer(std::shared_ptr<const UniformBlockInfo> ubi) :
    GlBuffer(VertexBufferType::UniformBuffer), m_uniformBlockInfo(std::move(ubi)), m_bindingPoint(0)
{
    constexpr std::byte* emptyData = nullptr;
    SetDataRaw(std::span{emptyData, m_uniformBlockInfo->DataSize});
}

void GlUniformBuffer::SetUniform(std::string_view name, const Uniform& value)
{
    using namespace glm;

    std::visit([&](const auto& x)
    {
        if (auto* field = m_uniformBlockInfo->Layout[name])
        {
            // All buffer is mapping just for one variable - very inefficient, but only writer knows real field size.
            // Besides, this code will be removed very soon
            DataIO::Write(*field, Map(AT2::BufferUsage::ReadWrite).subspan(field->GetOffset()), x); 
            Unmap();
        }
    }, value);
}

void GlUniformBuffer::Bind(IStateManager&) const
{
    glBindBufferBase(GL_UNIFORM_BUFFER, m_bindingPoint, GetId());
}


void GlUniformBuffer::SetUniform(std::string_view, const std::shared_ptr<ITexture>&)
{
    throw std::logic_error("not implemented");
}