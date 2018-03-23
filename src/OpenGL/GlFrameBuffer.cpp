#include "GlFrameBuffer.h"

using namespace AT2;

GlFrameBuffer::GlFrameBuffer(const IRendererCapabilities& rendererCapabilities) :
	m_size(0, 0),
	m_colorAttachements(rendererCapabilities.GetMaxNumberOfColorAttachements())
	//m_attachementsView(m_colorAttachements, [&](size_t i, std::shared_ptr<GlTexture> texture){BindAttachement(i, texture); }, [](size_t i, std::shared_ptr<GlTexture>){}, [&](size_t i, std::shared_ptr<GlTexture>){UnbindAttachement(i); })
{
	glGenFramebuffers(1, &m_id);
}

GlFrameBuffer::~GlFrameBuffer()
{
	glDeleteFramebuffers(1, &m_id);
}

void GlFrameBuffer::SetColorAttachement(unsigned int attachementNumber, const std::shared_ptr<GlTexture> texture)
{
	if (attachementNumber >= m_colorAttachements.size())
		throw AT2Exception(AT2Exception::ErrorCase::Buffer, "GlFrameBuffer: unsupported attachement number");

	auto attachement = GL_COLOR_ATTACHMENT0 + attachementNumber;
	if (texture)
	{
		if (dynamic_cast<GlTexture1D*>(texture.get()) != nullptr)
			glNamedFramebufferTexture1DEXT(m_id, attachement, static_cast<GLenum>(texture->GetTargetType()), texture->GetId(), 0); //I don't know practical purpose of it, but let it be =)
		else if (dynamic_cast<GlTexture2D*>(texture.get()) != nullptr)
			glNamedFramebufferTexture2DEXT(m_id, attachement, static_cast<GLenum>(texture->GetTargetType()), texture->GetId(), 0);
		else if (dynamic_cast<GlTexture3D*>(texture.get()) != nullptr)
			glNamedFramebufferTexture3DEXT(m_id, attachement, static_cast<GLenum>(texture->GetTargetType()), texture->GetId(), 0, 0); //TODO: zoffset controlling
		else
			return;

		m_colorAttachements[attachementNumber] = texture;
	}
	else
	{
		glNamedFramebufferTextureEXT(m_id, attachement, 0, 0);
	}

	Validate();
}

std::shared_ptr<ITexture> GlFrameBuffer::GetColorAttachement(unsigned int attachementNumber) const
{
	if (attachementNumber >= m_colorAttachements.size())
		throw AT2Exception(AT2Exception::ErrorCase::Buffer, "GlFrameBuffer: unsupported attachement number");

	return m_colorAttachements[attachementNumber];
}

void GlFrameBuffer::SetDepthAttachement(const std::shared_ptr<GlTexture> texture)
{
	m_depthAttachement = texture;
	if (texture)
		glNamedFramebufferTexture2DEXT(m_id, GL_DEPTH_ATTACHMENT, static_cast<GLenum>(texture->GetTargetType()), texture->GetId(), 0);
	else
		glNamedFramebufferTexture2DEXT(m_id, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);

	Validate();
}

std::shared_ptr<ITexture> GlFrameBuffer::GetDepthAttachement() const
{
	return m_depthAttachement;
}

void GlFrameBuffer::Validate()
{
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		throw AT2Exception(AT2Exception::ErrorCase::Buffer, "GlFrameBuffer: framebuffer validation failed");
}


void GlFrameBuffer::Resize (const glm::ivec2& _size)
{
	/*
	glGenRenderbuffers (1, &renderbufferId );
	glNamedRenderbufferStorageEXT (renderbufferId, GL_DEPTH_COMPONENT, _size.x, _size.y );
	glNamedFramebufferRenderbufferEXT (m_id, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbufferId );


	glNamedFramebufferTexture2DEXT (m_id, GL_COLOR_ATTACHMENT0, target, texId, 0);

	if (glCheckNamedFramebufferStatusEXT (m_id, static_cast<GLenum>(m_type)) != GL_FRAMEBUFFER_COMPLETE)
		throw AT2Exception(AT2Exception::ErrorCase::Buffer, "GlFrameBuffer: framebuffer is incorrect");
	*/
	m_size = _size;
}

void GlFrameBuffer::Bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_id);

	const size_t numAttachements = m_colorAttachements.size();
	Utils::dynarray<GLenum> buffers(numAttachements);
	for (size_t i = 0; i < numAttachements; ++i)
		buffers[i] = (m_colorAttachements[i]) ? GL_COLOR_ATTACHMENT0 + i : GL_NONE;

	glDrawBuffers(buffers.size(), buffers.data());
}