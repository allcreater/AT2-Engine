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

	void SetColorAttachement(unsigned int attachementNumber, const std::shared_ptr<ITexture>& texture) override;
	std::shared_ptr<ITexture> GetColorAttachement(unsigned int attachementNumber) const override;
	void SetDepthAttachement(const std::shared_ptr<ITexture>& texture) override;
	std::shared_ptr<ITexture> GetDepthAttachement() const override;
	glm::ivec2 GetActualSize() const override { return m_size; }

private:
	GLuint m_id;

	glm::ivec2 m_size {0, 0};

	std::vector<std::shared_ptr<GlTexture>> m_colorAttachements;
	std::shared_ptr<GlTexture> m_depthAttachement;

	bool m_dirtyFlag { true };
};

class GlScreenFrameBuffer : public IFrameBuffer
{
	GlScreenFrameBuffer() = default;

public:
	static GlScreenFrameBuffer& Get()
	{
		static GlScreenFrameBuffer defaultFB;
		return defaultFB;
	}

	void Bind() override
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	unsigned int GetId() const override
	{
		return 0;
	}

	void SetColorAttachement(unsigned int attachementNumber, const std::shared_ptr<ITexture>& texture) override
	{
		throw AT2Exception(AT2Exception::ErrorCase::NotImplemented, "GlScreenFrameBuffer dont'support attachements");
	}
	std::shared_ptr<ITexture> GetColorAttachement(unsigned int attachementNumber) const override
	{
		throw AT2Exception(AT2Exception::ErrorCase::NotImplemented, "GlScreenFrameBuffer dont'support attachements");
	}
	void SetDepthAttachement(const std::shared_ptr<ITexture>& texture) override
	{
		throw AT2Exception(AT2Exception::ErrorCase::NotImplemented, "GlScreenFrameBuffer dont'support attachements");
	}
	std::shared_ptr<ITexture> GetDepthAttachement() const override
	{
		throw AT2Exception(AT2Exception::ErrorCase::NotImplemented, "GlScreenFrameBuffer dont'support attachements");
	}

	glm::ivec2 GetActualSize() const
	{
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);

		return { viewport[2] - viewport[0], viewport[3] - viewport[1] };
	}
};

}
#endif