#ifndef AT2_GL_UNIFORMBUFFER_H
#define AT2_GL_UNIFORMBUFFER_H

#include "GlShaderProgram.h"
#include "GlVertexBuffer.h"

namespace AT2
{

    class GlUniformBuffer : public GlVertexBuffer, public IUniformContainer
    {
    public:
        NON_COPYABLE_OR_MOVABLE(GlUniformBuffer)

        GlUniformBuffer(std::shared_ptr<GlShaderProgram::UniformBufferInfo> ubi);
        ~GlUniformBuffer() override;

    public:
        void SetUniform(const str& name, const Uniform& value) override;

        //textures
        void SetUniform(const str& name, const std::shared_ptr<ITexture>& value) override;

        void Bind(IStateManager& stateManager) const override;
        void SetBindingPoint(unsigned int index) { m_bindingPoint = index; }

    private:
        std::shared_ptr<GlShaderProgram::UniformBufferInfo> m_ubi;
        GLuint m_bindingPoint;
    };

} // namespace AT2

#endif