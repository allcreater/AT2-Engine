#ifndef AT2_GL_FRAMEBUFFER_H
#define AT2_GL_FRAMEBUFFER_H

#include "AT2lowlevel.h"

#include "GlTexture.h"

namespace AT2::OpenGL
{
    class GlRenderer;

    class GlFrameBuffer : public IFrameBuffer
    {
    public:
        NON_COPYABLE_OR_MOVABLE(GlFrameBuffer)

        GlFrameBuffer(GlRenderer& renderer);
        ~GlFrameBuffer() override;

    public:
        void SetColorAttachment(unsigned int attachmentNumber, std::shared_ptr<ITexture> attachment) override;
        [[nodiscard]] ColorAttachment GetColorAttachment(unsigned int attachmentNumber) const override;
        void SetDepthAttachment(DepthAttachment attachment) override;
        [[nodiscard]] DepthAttachment GetDepthAttachment() const override;

        void SetClearColor(std::optional<glm::vec4> color) override;
        void SetClearColor(unsigned int attachmentNumber, std::optional<glm::vec4> color) override;
        void SetClearDepth(std::optional<float> depth) override;

        [[nodiscard]] glm::uvec2 GetActualSize() const noexcept override { return m_size; }

        void Render(RenderFunc renderFunc) override;

    private:
        GlRenderer& m_renderer;
        GLuint m_id;

        glm::uvec2 m_size {0, 0};

        std::vector<ColorAttachment> m_colorAttachments;
        DepthAttachment m_depthAttachment;

        bool m_dirtyFlag {true};

    };

    class GlScreenFrameBuffer : public IFrameBuffer
    {
    public:
        NON_COPYABLE_OR_MOVABLE(GlScreenFrameBuffer)

        GlScreenFrameBuffer(GlRenderer &renderer);

        void SetColorAttachment(unsigned int attachmentNumber, std::shared_ptr<ITexture> attachment) override;
        [[nodiscard]] ColorAttachment GetColorAttachment(unsigned int attachmentNumber) const override;
        void SetDepthAttachment(DepthAttachment attachment) override;
        [[nodiscard]] DepthAttachment GetDepthAttachment() const override;

        void SetClearColor(std::optional<glm::vec4> color) override;
        void SetClearColor(unsigned int attachmentNumber, std::optional<glm::vec4> color) override;
        void SetClearDepth(std::optional<float> depth) override;

        [[nodiscard]] glm::uvec2 GetActualSize() const override;

        void Render(RenderFunc renderFunc) override;

    private:
        GlRenderer& m_renderer;
        std::optional<glm::vec4> m_clearColor;
        std::optional<float> m_clearDepth;
    };

} // namespace AT2::OpenGL
#endif
