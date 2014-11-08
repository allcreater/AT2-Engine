#ifndef AT2_GL_FRAMEBUFFER_H
#define AT2_GL_FRAMEBUFFER_H

#include "AT2lowlevel.h"

#include "GlTexture.h"

namespace AT2
{

class GlFrameBuffer : public IFrameBuffer
{
public:
	GlFrameBuffer();
	~GlFrameBuffer();

public:
	virtual void Bind();
	virtual unsigned int GetId() const { return m_id; }

	virtual void Resize (const glm::ivec2& size);

	virtual void BindColorAttachement(unsigned int attachementNumber, const std::shared_ptr<GlTexture> texture);
	virtual void BindDepthAttachement(const std::shared_ptr<GlTexture> texture);

private:
	GLuint m_id;

	glm::ivec2 m_size;
};

}
#endif