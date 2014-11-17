#ifndef AT2_GL_UNIFORMBUFFER_H
#define AT2_GL_UNIFORMBUFFER_H

#include "GlVertexBuffer.h"
#include "GlShaderProgram.h"

namespace AT2
{

class GlUniformBuffer : public GlVertexBufferBase
{
public:
	GlUniformBuffer(const GlShaderProgram& shader, const str& blockName);
	~GlUniformBuffer();

public:
	virtual GlBufferType GetType() const { return GlBufferType::UniformBuffer; }

	virtual void Bind();

private:
	std::shared_ptr<GlShaderProgram::UniformBufferInfo> m_ubi;
};

}

#endif