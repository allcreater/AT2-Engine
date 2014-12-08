#ifndef AT2_GL_FRAMEBUFFER_H
#define AT2_GL_FRAMEBUFFER_H

#include "AT2lowlevel.h"

#include "GlTexture.h"

namespace AT2
{

class GlFrameBuffer : public IFrameBuffer
{
public:
	GlFrameBuffer(IRendererCapabilities* rendererCapabilities);
	~GlFrameBuffer();

public:
	virtual void Bind();
	virtual unsigned int GetId() const { return m_id; }

	virtual void Resize (const glm::ivec2& size);

	//virtual void BindColorAttachement(unsigned int attachementNumber, const std::shared_ptr<GlTexture> texture);
	virtual void BindDepthAttachement(const std::shared_ptr<GlTexture> texture);

	Utils::ControlledList<Utils::dynarray<std::shared_ptr<GlTexture>>, std::shared_ptr<GlTexture>>& GetColorAttachementsList()
	{
		return m_attachementsView; 
	}
private:
	void BindAttachement(unsigned int number, const std::shared_ptr<GlTexture> texture);
	void UnbindAttachement(unsigned int number);

private:
	GLuint m_id;

	glm::ivec2 m_size;

	Utils::dynarray<std::shared_ptr<GlTexture>> m_colorAttachements;
	Utils::ControlledList<Utils::dynarray<std::shared_ptr<GlTexture>>, std::shared_ptr<GlTexture>> m_attachementsView;
};

class GlScreenFrameBuffer : public IFrameBuffer
{
public:
	virtual void Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	virtual unsigned int GetId() const
	{
		return 0;
	}
};

}
#endif