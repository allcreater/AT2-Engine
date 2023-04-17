#include "GlFrameBuffer.h"
#include "GlRenderer.h"

using namespace AT2;
using namespace AT2::OpenGL41;

namespace
{
    void ResetViewport(IFrameBuffer& self)
    {
        glDisable(GL_SCISSOR_TEST); //TODO: apply via StateManager?

        const auto actualViewportSize = self.GetActualSize();
        glViewport(0, 0, actualViewportSize.x, actualViewportSize.y);
    }
}

GlFrameBuffer::GlFrameBuffer(GlRenderer& renderer)
    : m_renderer(renderer)
    , m_colorAttachments(renderer.GetRendererCapabilities().GetMaxNumberOfColorAttachments())
{
    glCreateFramebuffers(1, &m_id);
}

GlFrameBuffer::~GlFrameBuffer()
{
    glDeleteFramebuffers(1, &m_id);
}

void GlFrameBuffer::SetColorAttachment(unsigned int attachmentNumber, std::shared_ptr<ITexture> attachment)
{
    if (attachmentNumber >= m_colorAttachments.size())
        throw AT2BufferException( "GlFrameBuffer: unsupported attachment number");

    m_dirtyFlag = true;

    auto glTexture = std::dynamic_pointer_cast<GlTexture>(attachment);
    const bool isError = attachment && !glTexture;

    const auto attachmentId = GL_COLOR_ATTACHMENT0 + attachmentNumber;
    if (glTexture)
    {
        std::visit(
            [=](const auto& type) {
                using T = std::decay_t<decltype(type)>;
                if constexpr (std::is_same_v<T, Texture1D> || std::is_same_v<T, Texture2D> ||
                              std::is_same_v<T, Texture3D>)
                    glNamedFramebufferTexture(m_id, attachmentId, glTexture->GetId(), 0);
            },
            glTexture->GetType());
    }
    else
    {
        glNamedFramebufferTexture(m_id, attachmentId, 0, 0);

        if (isError)
        {
            m_colorAttachments[attachmentNumber] = {};
            throw AT2BufferException(
                               "GlFrameBuffer: attachment texture should be GlTexture");
        }
    }

    m_colorAttachments[attachmentNumber].Texture = std::move(attachment);
}

IFrameBuffer::ColorAttachment GlFrameBuffer::GetColorAttachment(unsigned int attachmentNumber) const
{
    return m_colorAttachments.at(attachmentNumber);
}

void GlFrameBuffer::SetDepthAttachment(DepthAttachment attachment)
{
    m_dirtyFlag = true;

    const auto* glTexture = dynamic_cast<GlTexture*>(attachment.Texture.get());
    const bool isError = attachment.Texture && !glTexture;

    m_depthAttachment = attachment;
    if (!m_depthAttachment.Texture)
    {
        glNamedFramebufferTexture(m_id, GL_DEPTH_ATTACHMENT, 0, 0);
        if (isError)
            throw AT2BufferException(
                               "GlFrameBuffer: attachment texture should be GlTexture");
    }
    else if (std::holds_alternative<Texture2D>(glTexture->GetType()))
        glNamedFramebufferTexture(m_id, GL_DEPTH_ATTACHMENT, glTexture->GetId(), 0);
    else
        throw AT2BufferException( "GlFrameBuffer: depth attachment should be Texture2D");
}

IFrameBuffer::DepthAttachment GlFrameBuffer::GetDepthAttachment() const
{
    return m_depthAttachment;
}

void GlFrameBuffer::SetClearColor(std::optional<glm::vec4> color) 
{
    for (auto& attachment : m_colorAttachments)
        attachment.ClearColor = color;
}

void GlFrameBuffer::SetClearColor(unsigned int attachmentNumber, std::optional<glm::vec4> color)
{
    m_colorAttachments.at(attachmentNumber).ClearColor = std::move(color);
}

void GlFrameBuffer::SetClearDepth(std::optional<float> depth) 
{
    m_depthAttachment.ClearDepth = depth;
}

void GlFrameBuffer::Render(RenderFunc renderFunc) 
{
    ResetViewport(*this);
    
    glBindFramebuffer(GL_FRAMEBUFFER, m_id);

    //validating
    if (m_dirtyFlag)
    {
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            throw AT2BufferException("GlFrameBuffer: validation failed");

        m_size = m_colorAttachments[0].Texture->GetSize();
        m_dirtyFlag = false;
    }

    const auto numAttachments = m_colorAttachments.size();
    assert(m_colorAttachments.size() < (GL_MAX_COLOR_ATTACHMENTS - GL_COLOR_ATTACHMENT0));

    std::vector<GLenum> buffers(numAttachments);
    for (GLenum i = 0; i < numAttachments; ++i)
        buffers[i] = (m_colorAttachments[i].Texture) ? GL_COLOR_ATTACHMENT0 + i : GL_NONE;
    glDrawBuffers(static_cast<GLsizei>(buffers.size()), buffers.data());


    // clearing buffers
    for (size_t i = 0; i < numAttachments; ++i)
    {
        if (m_colorAttachments[i].ClearColor)
            glClearBufferfv(GL_COLOR, static_cast<GLint>(i), glm::value_ptr(m_colorAttachments[i].ClearColor.value()));
    }

    if (m_depthAttachment.ClearDepth)
    {
        const auto depth = static_cast<float>(m_depthAttachment.ClearDepth.value()); //only float is supported :(
        glClearBufferfv(GL_DEPTH, 0, &depth);
    }

    renderFunc(m_renderer);
}


// GlScreenFrameBuffer

GlScreenFrameBuffer::GlScreenFrameBuffer(GlRenderer& renderer) : m_renderer(renderer) {}

void GlScreenFrameBuffer::SetColorAttachment(unsigned int attachmentNumber, std::shared_ptr<ITexture> attachment)
{
    throw AT2NotImplementedException("GlScreenFrameBuffer dont'support attachements");
}

[[nodiscard]] IFrameBuffer::ColorAttachment GlScreenFrameBuffer::GetColorAttachment(unsigned int attachmentNumber) const
{
    throw AT2NotImplementedException("GlScreenFrameBuffer dont'support attachements");
}
void GlScreenFrameBuffer::SetDepthAttachment(DepthAttachment attachment)
{
    throw AT2NotImplementedException("GlScreenFrameBuffer dont'support attachements");
}

[[nodiscard]] IFrameBuffer::DepthAttachment GlScreenFrameBuffer::GetDepthAttachment() const
{
    throw AT2NotImplementedException("GlScreenFrameBuffer dont'support attachements");
}

void GlScreenFrameBuffer::SetClearColor(std::optional<glm::vec4> color) 
{
    m_clearColor = color;
}

void GlScreenFrameBuffer::SetClearColor(unsigned int attachmentNumber, std::optional<glm::vec4> color)
{
    SetClearColor(std::move(color));
}

void GlScreenFrameBuffer::SetClearDepth(std::optional<float> depth) 
{
    m_clearDepth = depth;
}

[[nodiscard]] glm::uvec2 GlScreenFrameBuffer::GetActualSize() const
{
    return m_renderer.GetGraphicsContext().getPhysicalViewportSize();
}

void GlScreenFrameBuffer::Render(RenderFunc renderFunc) 
{
    ResetViewport(*this);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (m_clearColor)
        glClearColor(m_clearColor->r, m_clearColor->g, m_clearColor->b, m_clearColor->a);

    if (m_clearDepth)
        glClearDepth(m_clearDepth.value());

    glClear(GL_COLOR_BUFFER_BIT * m_clearColor.has_value() | GL_DEPTH_BUFFER_BIT * m_clearDepth.has_value());

    renderFunc(m_renderer);
}
