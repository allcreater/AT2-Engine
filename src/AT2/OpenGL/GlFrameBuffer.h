#ifndef AT2_GL_FRAMEBUFFER_H
#define AT2_GL_FRAMEBUFFER_H

#include "AT2lowlevel.h"

#include "GlTexture.h"

namespace AT2
{

class GlFrameBuffer : public IFrameBuffer
{
public:
	NON_COPYABLE_OR_MOVABLE(GlFrameBuffer)

	GlFrameBuffer(const IRendererCapabilities& rendererCapabilities);
	~GlFrameBuffer() override;

public:
	void Bind() override;
	unsigned int GetId() const override { return m_id; }

	void SetColorAttachment(unsigned int attachmentNumber, const std::shared_ptr<ITexture>& texture) override;
    std::shared_ptr<ITexture> GetColorAttachment(unsigned int attachmentNumber) const override;
	void SetDepthAttachment(const std::shared_ptr<ITexture>& texture) override;
    std::shared_ptr<ITexture> GetDepthAttachment() const override;
	glm::ivec2 GetActualSize() const noexcept override { return m_size; }

private:
	GLuint m_id;

	glm::ivec2 m_size {0, 0};

	std::vector<std::shared_ptr<GlTexture>> m_colorAttachments;
	std::shared_ptr<GlTexture> m_depthAttachment;

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

	void SetColorAttachment(unsigned int attachementNumber, const std::shared_ptr<ITexture>& texture) override
	{
		throw AT2Exception(AT2Exception::ErrorCase::NotImplemented, "GlScreenFrameBuffer dont'support attachements");
	}

    std::shared_ptr<ITexture> GetColorAttachment(unsigned int attachementNumber) const override
	{
		throw AT2Exception(AT2Exception::ErrorCase::NotImplemented, "GlScreenFrameBuffer dont'support attachements");
	}
	void SetDepthAttachment(const std::shared_ptr<ITexture>& texture) override
	{
		throw AT2Exception(AT2Exception::ErrorCase::NotImplemented, "GlScreenFrameBuffer dont'support attachements");
	}

    std::shared_ptr<ITexture> GetDepthAttachment() const override
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