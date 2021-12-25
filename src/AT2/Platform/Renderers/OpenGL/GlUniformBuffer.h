#ifndef AT2_GL_UNIFORMBUFFER_H
#define AT2_GL_UNIFORMBUFFER_H

#include "GlShaderProgram.h"
#include "GlVertexBuffer.h"

namespace AT2::OpenGL
{

    class GlUniformBuffer : public GlVertexBuffer, public IUniformContainer
    {
    public:
        NON_COPYABLE_OR_MOVABLE(GlUniformBuffer)

        //UniformBlockInfo does not linked with specific program, it's just template for buffer structuring
        //TODO: Abstract interface to uniform buffer template definition. It should not be linked with program introspection directly.
        explicit GlUniformBuffer(std::shared_ptr<const OpenGL::Introspection::UniformBlockInfo> ubi);

    public:
        void SetUniform(std::string_view name, const Uniform& value) override;

        //textures
        void SetUniform(std::string_view name, const std::shared_ptr<ITexture>& value) override;

        //TODO:  there should not be VertexBuffer::Bind(), it restricts Liskov's substitution principle =(
        void Bind(IStateManager& stateManager) const override;
        void SetBindingPoint(unsigned int index) { m_bindingPoint = index; }

    private:
        std::shared_ptr<const OpenGL::Introspection::UniformBlockInfo> m_uniformBlockInfo;
        std::string m_blockName;
        GLuint m_bindingPoint;
    };

} // namespace AT2

#endif