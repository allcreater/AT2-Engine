#ifndef AT2_GL_FRAMEBUFFER_H
#define AT2_GL_FRAMEBUFFER_H

#include "AT2lowlevel.h"

#include "GlTexture.h"

namespace AT2
{

class GlFrameBuffer : public IFrameBuffer
{
public:
	GlFrameBuffer(const IRendererCapabilities& rendererCapabilities);
	~GlFrameBuffer();

public:
	void Bind() override;
	unsigned int GetId() const override { return m_id; }

	virtual void Resize (const glm::ivec2& size);

	void SetColorAttachement(unsigned int attachementNumber, std::shared_ptr<ITexture> texture) override;
	std::shared_ptr<ITexture> GetColorAttachement(unsigned int attachementNumber) const override;
	void SetDepthAttachement(const std::shared_ptr<ITexture> texture) override;
	std::shared_ptr<ITexture> GetDepthAttachement() const override;

private:
	void Validate();

private:
	GLuint m_id;

	glm::ivec2 m_size;

	Utils::dynarray<std::shared_ptr<GlTexture>> m_colorAttachements;
	std::shared_ptr<GlTexture> m_depthAttachement;

	//Utils::ControlledList<Utils::dynarray<std::shared_ptr<GlTexture>>, std::shared_ptr<GlTexture>> m_attachementsView;
};

class GlScreenFrameBuffer : public IFrameBuffer
{
public:
	void Bind() override
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	unsigned int GetId() const override
	{
		return 0;
	}

	void SetColorAttachement(unsigned int attachementNumber, std::shared_ptr<ITexture> texture) override
	{
		throw AT2Exception(AT2Exception::ErrorCase::NotImplemented, "GlScreenFrameBuffer dont'support attachements");
	}
	std::shared_ptr<ITexture> GetColorAttachement(unsigned int attachementNumber) const override
	{
		throw AT2Exception(AT2Exception::ErrorCase::NotImplemented, "GlScreenFrameBuffer dont'support attachements");
	}
	void SetDepthAttachement(const std::shared_ptr<ITexture> texture) override
	{
		throw AT2Exception(AT2Exception::ErrorCase::NotImplemented, "GlScreenFrameBuffer dont'support attachements");
	}
	std::shared_ptr<ITexture> GetDepthAttachement() const override
	{
		throw AT2Exception(AT2Exception::ErrorCase::NotImplemented, "GlScreenFrameBuffer dont'support attachements");
	}
};

}
#endif