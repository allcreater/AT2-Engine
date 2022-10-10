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

        FrameBuffer(Renderer& renderer, size_t maxAttachments);

    public:
        void SetColorAttachment(unsigned int attachmentNumber, std::shared_ptr<ITexture> attachment) override;
        [[nodiscard]] ColorAttachment GetColorAttachment(unsigned int attachmentNumber) const override;
        void SetDepthAttachment(DepthAttachment attachment) override;
        [[nodiscard]] DepthAttachment GetDepthAttachment() const override;

        void SetClearColor(unsigned int attachmentNumber, std::optional<glm::vec4> color) override;
        void SetClearColor(std::optional<glm::vec4> color) override;
        void SetClearDepth(std::optional<float> depth) override;

        [[nodiscard]] glm::uvec2 GetActualSize() const noexcept override { return m_size; }

        void Render(RenderFunc renderFunc) override;

    protected:
        virtual void OnCommit(MTL::CommandBuffer* commandBuffer){}
        
        MtlPtr<MTL::RenderPassDescriptor> m_renderPassDescriptor;
        
        void SetAttachmentTexture(MTL::RenderPassAttachmentDescriptor* attachment, MTL::Texture* nativeTexture);
        void SetAttachmentTexture(MTL::RenderPassAttachmentDescriptor* attachment, ITexture* texture);
        static void SetAttachmentClearColor(MTL::RenderPassColorAttachmentDescriptor* attachment, const std::optional<glm::vec4>& clearColor);
        static void SetAttachmentClearDepth(MTL::RenderPassDepthAttachmentDescriptor* attachment, const std::optional<float>& clearDepth);
        
        Renderer& GetVisualisationSystem() const noexcept { return m_renderer; }
        
    private:
        Renderer& m_renderer;
        glm::uvec2 m_size {0, 0};

        std::vector<ColorAttachment> m_colorAttachments;
        DepthAttachment m_depthAttachment;
    };

    //By default screen frame buffer will create depth texture automatically. It's Float32.
    class MetalScreenFrameBuffer : public FrameBuffer
    {
    public:
        NON_COPYABLE_OR_MOVABLE(MetalScreenFrameBuffer)
        
        MetalScreenFrameBuffer(Renderer& renderer, CA::MetalLayer* swapChain) : FrameBuffer{renderer, 1}, m_swapChain{swapChain} {}
        
        void Render(RenderFunc renderFunc) override;
        
    private:
        void OnCommit(MTL::CommandBuffer* commandBuffer) override;
        
        void PrepareAttachments();

    private:
        MtlPtr<CA::MetalLayer> m_swapChain;
        CA::MetalDrawable* m_drawable;

    };

} // namespace AT2::Metal
