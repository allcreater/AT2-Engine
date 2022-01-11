#pragma once

#include "AT2lowlevel.h"

#include "Texture.h"

namespace AT2::Metal
{
    class FrameBuffer : public IFrameBuffer
    {
    public:
        NON_COPYABLE_OR_MOVABLE(FrameBuffer)

        FrameBuffer(const IRendererCapabilities& rendererCapabilities);
        ~FrameBuffer() override;

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
        glm::ivec2 m_size {0, 0};

        std::vector<std::optional<ColorAttachment>> m_colorAttachments;
        DepthAttachment m_depthAttachment;

        bool m_dirtyFlag {true};
    };

    class MetalScreenFrameBuffer : public IFrameBuffer
    {
        MetalScreenFrameBuffer() = default;

    public:
        static MetalScreenFrameBuffer& Get()
        {
            static MetalScreenFrameBuffer defaultFB;
            return defaultFB;
        }

        [[nodiscard]] unsigned int GetId() const noexcept override { return 0; }

        void SetColorAttachment(unsigned int, const std::shared_ptr<ITexture>&) override
        {
            throw AT2NotImplementedException("GlScreenFrameBuffer dont'support attachements");
        }

        [[nodiscard]] std::shared_ptr<ITexture> GetColorAttachment(unsigned int) const override
        {
            throw AT2NotImplementedException("GlScreenFrameBuffer dont'support attachements");
        }
        void SetDepthAttachment(const std::shared_ptr<ITexture>&) override
        {
            throw AT2NotImplementedException("GlScreenFrameBuffer dont'support attachements");
        }

        [[nodiscard]] std::shared_ptr<ITexture> GetDepthAttachment() const override
        {
            throw AT2NotImplementedException("GlScreenFrameBuffer dont'support attachements");
        }

        [[nodiscard]] glm::ivec2 GetActualSize() const override
        {
            return {0, 0};
        }
    };

} // namespace AT2::Metal
