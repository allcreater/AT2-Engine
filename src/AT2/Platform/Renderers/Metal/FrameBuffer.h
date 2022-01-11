#pragma once

#include "AT2lowlevel.h"

#include "Texture.h"

namespace AT2::Metal
{
    class Renderer;

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
    public:
        MetalScreenFrameBuffer(Renderer& renderer, CA::MetalLayer* swapChain) : m_renderer{renderer}, m_swapChain{swapChain} {}

        void SetColorAttachment(unsigned int attachmentNumber, ColorAttachment attachment) override
        {
            throw AT2NotImplementedException("GlScreenFrameBuffer dont'support attachements");
        }

        const ColorAttachment* GetColorAttachment(unsigned int attachmentNumber) const override
        {
            throw AT2NotImplementedException("GlScreenFrameBuffer dont'support attachements");
        }
        void SetDepthAttachment(DepthAttachment attachment) override
        {
            throw AT2NotImplementedException("GlScreenFrameBuffer dont'support attachements");
        }

        [[nodiscard]] const DepthAttachment* GetDepthAttachment() const override
        {
            throw AT2NotImplementedException("GlScreenFrameBuffer dont'support attachements");
        }

        [[nodiscard]] glm::ivec2 GetActualSize() const override
        {
            return {0, 0};
        }
        
        void SetClearColor(std::optional<glm::vec4> color) override;
        void SetClearDepth(std::optional<float> depth) override;

        void Render(RenderFunc renderFunc) override;
        
    private:
        Renderer& m_renderer;
        MtlPtr<CA::MetalLayer> m_swapChain;
        
        std::optional<glm::vec4> m_clearColor;
        std::optional<float> m_clearDepth;
        
        struct FrameContext
        {
            MtlPtr<CA::MetalDrawable> drawable;
            MtlPtr<MTL::CommandBuffer> commandBuffer;
            MtlPtr<MTL::RenderCommandEncoder> renderEncoder;
        };
    };

} // namespace AT2::Metal
