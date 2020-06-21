#include "GlFrameBuffer.h"

using namespace AT2;

GlFrameBuffer::GlFrameBuffer(const IRendererCapabilities& rendererCapabilities) :
	m_colorAttachments(rendererCapabilities.GetMaxNumberOfColorAttachments())
{
	glGenFramebuffers(1, &m_id);
}

GlFrameBuffer::~GlFrameBuffer()
{
	glDeleteFramebuffers(1, &m_id);
}

void GlFrameBuffer::SetColorAttachment(unsigned int attachmentNumber, const std::shared_ptr<ITexture>& abstractTexture)
{
	if (attachmentNumber >= m_colorAttachments.size())
		throw AT2Exception(AT2Exception::ErrorCase::Buffer, "GlFrameBuffer: unsupported attachment number");

	m_dirtyFlag = true;

	auto texture = std::dynamic_pointer_cast<GlTexture>(abstractTexture);

	const auto attachement = GL_COLOR_ATTACHMENT0 + attachmentNumber;
	if (texture)
	{
		std::visit(Utils::overloaded {
     [=](const Texture1D& type)
            {
				glNamedFramebufferTexture1DEXT(m_id, attachement, texture->GetTarget(), texture->GetId(), 0); //I don't know practical purpose of it, but let it be =)
            },
	 [=](const Texture2D& type)
			{
				glNamedFramebufferTexture2DEXT(m_id, attachement, static_cast<GLenum>(texture->GetTarget()), texture->GetId(), 0);
			},
	 [=](const Texture3D& type)
			{
				glNamedFramebufferTexture3DEXT(m_id, attachement, static_cast<GLenum>(texture->GetTarget()), texture->GetId(), 0, 0); //TODO: zoffset controlling
			},
	 [=](auto&& type) //do nothing
			{
			}
	    }, texture->GetType());

	}
	else
	{
		glNamedFramebufferTextureEXT(m_id, attachement, 0, 0);

		if (abstractTexture)
		{
			m_colorAttachments[attachmentNumber] = nullptr;
			throw AT2Exception(AT2Exception::ErrorCase::Buffer, "GlFrameBuffer: attachment texture should be GlTexture");
		}
	}

    m_colorAttachments[attachmentNumber] = std::move(texture);
}

std::shared_ptr<ITexture> GlFrameBuffer::GetColorAttachment(unsigned int attachmentNumber) const
{
	if (attachmentNumber >= m_colorAttachments.size())
		throw AT2Exception(AT2Exception::ErrorCase::Buffer, "GlFrameBuffer: unsupported attachment number");

	return m_colorAttachments[attachmentNumber];
}

void GlFrameBuffer::SetDepthAttachment(const std::shared_ptr<ITexture>& abstractTexture)
{
	m_dirtyFlag = true;

	m_depthAttachment = std::dynamic_pointer_cast<GlTexture>(abstractTexture);
	if (!m_depthAttachment)
	{
		glNamedFramebufferTexture2DEXT(m_id, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
		if (abstractTexture)
			throw AT2Exception(AT2Exception::ErrorCase::Buffer, "GlFrameBuffer: attachment texture should be GlTexture");
	}
	else if (std::holds_alternative<Texture2D>(m_depthAttachment->GetType()))
		glNamedFramebufferTexture2DEXT(m_id, GL_DEPTH_ATTACHMENT, static_cast<GLenum>(m_depthAttachment->GetTarget()), m_depthAttachment->GetId(), 0);
	else
		throw AT2Exception(AT2Exception::ErrorCase::Buffer, "GlFrameBuffer: depth attachment should be Texture2D");

}

std::shared_ptr<ITexture> GlFrameBuffer::GetDepthAttachment() const
{
    return m_depthAttachment;
}

void GlFrameBuffer::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_id);

    //validating
    if(m_dirtyFlag)
    {
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            throw AT2Exception(AT2Exception::ErrorCase::Buffer, "GlFrameBuffer: validation failed");

        m_size = m_colorAttachments[0]->GetSize();
        m_dirtyFlag = false;
    }

    const auto numAttachments = m_colorAttachments.size();
    std::vector<GLenum> buffers (numAttachments);
    for (GLenum i = 0; i < numAttachments; ++i)
        buffers[i] = (m_colorAttachments[i]) ? GL_COLOR_ATTACHMENT0 + i : GL_NONE;


    glDrawBuffers(static_cast<GLsizei>(buffers.size()), buffers.data());
}
