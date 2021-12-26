#pragma once

#include "ShaderProgram.h"
#include "VertexBuffer.h"

namespace AT2::Metal
{

    class UniformBuffer : public VertexBuffer, public IUniformContainer
    {
    public:
        NON_COPYABLE_OR_MOVABLE(UniformBuffer)

        //UniformBlockInfo does not linked with specific program, it's just template for buffer structuring
        //TODO: Abstract interface to uniform buffer template definition. It should not be linked with program introspection directly.
        explicit UniformBuffer();

    public:
        void SetUniform(std::string_view name, const Uniform& value) override;

        //textures
        void SetUniform(std::string_view name, const std::shared_ptr<ITexture>& value) override;

        //TODO:  there should not be VertexBuffer::Bind(), it restricts Liskov's substitution principle =(
        void Bind(IStateManager& stateManager) const override;
        void SetBindingPoint(unsigned int index) { }

    private:
        std::string m_blockName;
    };

} // namespace AT2::Metal
