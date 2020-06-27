#ifndef AT2_GL_UNIFORMBUFFER_H
#define AT2_GL_UNIFORMBUFFER_H

#include "GlVertexBuffer.h"
#include "GlShaderProgram.h"

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
	void SetUniform(const str& name, std::weak_ptr<const ITexture> value) override;

	void Bind(IStateManager &stateManager) const override;
	void SetBindingPoint(unsigned int index) { m_bindingPoint = index; }

private:
	std::shared_ptr<GlShaderProgram::UniformBufferInfo> m_ubi;
	GLuint m_bindingPoint;
};

}

#endif