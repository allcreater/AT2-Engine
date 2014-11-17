#include "GlFrameBuffer.h"

using namespace AT2;

GlFrameBuffer::GlFrameBuffer() : m_size(0, 0)
{
	glGenFramebuffers(1, &m_id);
}

GlFrameBuffer::~GlFrameBuffer()
{
	glDeleteFramebuffers(1, &m_id);
}

void GlFrameBuffer::BindColorAttachement(unsigned int attachementNumber, const std::shared_ptr<GlTexture> texture)
{
	//TODO: incapsulate
	GLint maxColorAttachements = 0;
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColorAttachements);
	if (attachementNumber >= maxColorAttachements)
		throw AT2Exception(AT2Exception::ErrorCase::Buffer, "FrameBuffer: unsupported attachement number");
	
	if (dynamic_cast<GlTexture1D*>(texture.get()) != nullptr)
		glNamedFramebufferTexture1DEXT (m_id, GL_COLOR_ATTACHMENT0 + attachementNumber, static_cast<GLenum>(texture->GetTargetType()), texture->GetId(), 0); //I don't know practical purpose of it, but let it be =)
	else if (dynamic_cast<GlTexture2D*>(texture.get()) != nullptr)
		glNamedFramebufferTexture2DEXT (m_id, GL_COLOR_ATTACHMENT0 + attachementNumber, static_cast<GLenum>(texture->GetTargetType()), texture->GetId(), 0);
	else if (dynamic_cast<GlTexture3D*>(texture.get()) != nullptr)
		glNamedFramebufferTexture3DEXT (m_id, GL_COLOR_ATTACHMENT0 + attachementNumber, static_cast<GLenum>(texture->GetTargetType()), texture->GetId(), 0, 0); //TODO: zoffset controlling
}

void GlFrameBuffer::BindDepthAttachement(const std::shared_ptr<GlTexture> texture)
{
	glNamedFramebufferTexture2DEXT (m_id, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture->GetId(), 0);
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
}