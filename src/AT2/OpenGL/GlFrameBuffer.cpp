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

void GlFrameBuffer::SetColorAttachement(unsigned int attachementNumber, const std::shared_ptr<ITexture>& abstractTexture)
{
	if (attachementNumber >= m_colorAttachements.size())
		throw AT2Exception(AT2Exception::ErrorCase::Buffer, "GlFrameBuffer: unsupported attachement number");

	m_dirtyFlag = true;

	auto texture = std::dynamic_pointer_cast<GlTexture>(abstractTexture);

	const auto attachement = GL_COLOR_ATTACHMENT0 + attachementNumber;
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
			m_colorAttachements[attachementNumber] = nullptr;
			throw AT2Exception(AT2Exception::ErrorCase::Buffer, "GlFrameBuffer: attachement texture should be GlTexture");
		}
	}

    m_colorAttachements[attachementNumber] = std::move(texture);
}

std::shared_ptr<ITexture> GlFrameBuffer::GetColorAttachement(unsigned int attachementNumber) const
{
	if (attachementNumber >= m_colorAttachements.size())
		throw AT2Exception(AT2Exception::ErrorCase::Buffer, "GlFrameBuffer: unsupported attachement number");

	return m_colorAttachements[attachementNumber];
}

void GlFrameBuffer::SetDepthAttachement(const std::shared_ptr<ITexture>& abstractTexture)
{
	m_dirtyFlag = true;

	m_depthAttachement = std::dynamic_pointer_cast<GlTexture>(abstractTexture);
	if (!m_depthAttachement)
	{
		glNamedFramebufferTexture2DEXT(m_id, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
		if (abstractTexture)
			throw AT2Exception(AT2Exception::ErrorCase::Buffer, "GlFrameBuffer: attachement texture should be GlTexture");
	}
	else if (std::holds_alternative<Texture2D>(m_depthAttachement->GetType()))
		glNamedFramebufferTexture2DEXT(m_id, GL_DEPTH_ATTACHMENT, static_cast<GLenum>(m_depthAttachement->GetTarget()), m_depthAttachement->GetId(), 0);
	else
		throw AT2Exception(AT2Exception::ErrorCase::Buffer, "GlFrameBuffer: depth attachment should be Texture2D");

}

std::shared_ptr<ITexture> GlFrameBuffer::GetDepthAttachement() const
{
    return m_depthAttachement;
}

void GlFrameBuffer::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_id);

	//validating
    if(m_dirtyFlag)
    {
	    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		    throw AT2Exception(AT2Exception::ErrorCase::Buffer, "GlFrameBuffer: framebuffer validation failed");

	    m_size = m_colorAttachements[0]->GetSize();
	    m_dirtyFlag = false;
    }

    const auto numAttachements = m_colorAttachements.size();
    std::vector<GLenum> buffers (numAttachements);
    for (GLenum i = 0; i < numAttachements; ++i)
        buffers[i] = (m_colorAttachements[i]) ? GL_COLOR_ATTACHMENT0 + i : GL_NONE;


    glDrawBuffers(static_cast<GLsizei>(buffers.size()), buffers.data());
}
