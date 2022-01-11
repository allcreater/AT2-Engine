#ifndef AT2_GL_FRAMEBUFFER_H
#define AT2_GL_FRAMEBUFFER_H

#include "AT2lowlevel.h"

#include "GlTexture.h"

namespace AT2::OpenGL
{
    class GlFrameBuffer : public IFrameBuffer
    {
    public:
        NON_COPYABLE_OR_MOVABLE(GlFrameBuffer)

        GlFrameBuffer(IRenderer& renderer);
        ~GlFrameBuffer() override;

    public:
        void SetColorAttachment(unsigned int attachmentNumber, ColorAttachment attachment) override;
        [[nodiscard]] const ColorAttachment* GetColorAttachment(unsigned int attachmentNumber) const override;
        void SetDepthAttachment(DepthAttachment attachment) override;
        [[nodiscard]] const DepthAttachment* GetDepthAttachment() const override;

        void SetClearColor(std::optional<glm::vec4> color) override;
        void SetClearDepth(std::optional<float> depth) override;

        [[nodiscard]] glm::ivec2 GetActualSize() const noexcept override { return m_size; }

        void Render(RenderFunc renderFunc) override;

    private:
        IRenderer& m_renderer;
        GLuint m_id;

        glm::ivec2 m_size {0, 0};

        std::vector<std::optional<ColorAttachment>> m_colorAttachments;
        DepthAttachment m_depthAttachment;

        bool m_dirtyFlag {true};

    };

    class GlScreenFrameBuffer : public IFrameBuffer
    {
    public:
        NON_COPYABLE_OR_MOVABLE(GlScreenFrameBuffer)

        GlScreenFrameBuffer(IRenderer& renderer);

        void SetColorAttachment(unsigned int attachmentNumber, ColorAttachment attachment) override;

        [[nodiscard]] const ColorAttachment* GetColorAttachment(unsigned int attachmentNumber) const override;
        void SetDepthAttachment(DepthAttachment attachment) override;
        [[nodiscard]] const DepthAttachment* GetDepthAttachment() const override;

        void SetClearColor(std::optional<glm::vec4> color) override;
        void SetClearDepth(std::optional<float> depth) override;

        [[nodiscard]] glm::ivec2 GetActualSize() const override;

        void Render(RenderFunc renderFunc) override;

    private:
        IRenderer& m_renderer;
        std::optional<glm::vec4> m_clearColor;
        std::optional<float> m_clearDepth;
    };

} // namespace AT2::OpenGL
#endif