#ifndef AT2_GL_RENDERER_H
#define AT2_GL_RENDERER_H

#include "AT2lowlevel.h"
#include <GraphicsContextInterface.h>

namespace AT2::OpenGL41
{

    class GlRenderer;
    class GlTexture;
    class GlFrameBuffer;

    class GlResourceFactory : public IResourceFactory
    {
    public:
        NON_COPYABLE_OR_MOVABLE(GlResourceFactory)

        GlResourceFactory(GlRenderer& renderer);
        ~GlResourceFactory() override = default;

    public:
        std::shared_ptr<ITexture> CreateTextureFromFramebuffer(const glm::ivec2& pos,
                                                               const glm::uvec2& size) const override;
        std::shared_ptr<ITexture> CreateTexture(const Texture& declaration,
                                                ExternalTextureFormat desiredFormat) const override;
        std::shared_ptr<IFrameBuffer> CreateFrameBuffer() const override;
        std::shared_ptr<IVertexArray> CreateVertexArray() const override;
        std::shared_ptr<IBuffer> CreateBuffer(VertexBufferType type) const override;
        std::shared_ptr<IBuffer> CreateBuffer(VertexBufferType type, std::span<const std::byte> data) const override;
        std::shared_ptr<IShaderProgram> CreateShaderProgramFromFiles(std::initializer_list<str> files) const override;
        std::shared_ptr<IPipelineState> CreatePipelineState(const PipelineStateDescriptor& pipelineStateDescriptor) const override;
        
        void ReloadResources(ReloadableGroup group) override;

    private:
        GlRenderer& m_renderer;
        mutable std::vector<std::weak_ptr<IReloadable>> m_reloadableResourcesList;
    };

    class GlRenderer : public IVisualizationSystem, public IRenderer
    {
    public:
        NON_COPYABLE_OR_MOVABLE(GlRenderer)

        GlRenderer(IPlatformGraphicsContext& graphicsContext, GLADloadproc glFunctionsBinder);
        ~GlRenderer() override = default;

    public:
        [[nodiscard]] IResourceFactory& GetResourceFactory() const override { return *m_resourceFactory; }
        [[nodiscard]] IStateManager& GetStateManager() override { return *m_stateManager; }
        [[nodiscard]] IRendererCapabilities& GetRendererCapabilities() const override
        {
            return *m_rendererCapabilities;
        }

        void DispatchCompute(const std::shared_ptr<IShaderProgram>& computeProgram, glm::uvec3 threadGroupSize) override;
        void Draw(Primitives::Primitive type, size_t first, long int count, int numInstances = 1, int baseVertex = 0) override;

        void SetViewport(const AABB2d& viewport) override;
        void SetScissorWindow(const AABB2d& viewport) override;
        void BeginFrame() override;
        void FinishFrame() override;

        [[nodiscard]] IFrameBuffer& GetDefaultFramebuffer() const override;

        IVisualizationSystem& GetVisualizationSystem() override { return *this; }

        const IPlatformGraphicsContext& GetGraphicsContext() const { return m_graphicsContext; }

    private:
        static void __stdcall GlErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                    const GLchar* message, const GLvoid* userParam);

    private:
        IPlatformGraphicsContext& m_graphicsContext;
        std::unique_ptr<IStateManager> m_stateManager;
        std::unique_ptr<IResourceFactory> m_resourceFactory;
        std::unique_ptr<IRendererCapabilities> m_rendererCapabilities;
        std::unique_ptr<IFrameBuffer> m_defaultFramebuffer;
    };

} // namespace AT2::OpenGL41

#endif
