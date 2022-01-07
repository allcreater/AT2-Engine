#ifndef AT2_GL_RENDERER_H
#define AT2_GL_RENDERER_H

#include "AT2lowlevel.h"

namespace AT2::OpenGL
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
        std::shared_ptr<IVertexBuffer> CreateVertexBuffer(VertexBufferType type) const override;
        std::shared_ptr<IVertexBuffer> CreateVertexBuffer(VertexBufferType type, std::span<const std::byte> data) const override;
        std::shared_ptr<IShaderProgram> CreateShaderProgramFromFiles(std::initializer_list<str> files) const override;
        void ReloadResources(ReloadableGroup group) override;

    private:
        GlRenderer& m_renderer;
        mutable std::vector<std::weak_ptr<IReloadable>> m_reloadableResourcesList;
    };

    class GlRenderer : public IRenderer
    {
    public:
        NON_COPYABLE_OR_MOVABLE(GlRenderer)

        GlRenderer(GLADloadproc glFunctionsBinder);
        ~GlRenderer() override = default;

    public:
        [[nodiscard]] IResourceFactory& GetResourceFactory() const override { return *m_resourceFactory; }
        [[nodiscard]] IStateManager& GetStateManager() const override { return *m_stateManager; }
        [[nodiscard]] IRendererCapabilities& GetRendererCapabilities() const override
        {
            return *m_rendererCapabilities;
        }

        void Shutdown() override;

        void DispatchCompute(glm::uvec3 threadGroupSize) override;
        void Draw(Primitives::Primitive type, size_t first, long int count, int numInstances = 1,
                  int baseVertex = 0) override;

        void SetViewport(const AABB2d& viewport) override;
        void ClearBuffer(const glm::vec4& color) override;
        void ClearDepth(float depth) override;
        void BeginFrame() override;
        void FinishFrame() override;

        [[nodiscard]] IFrameBuffer& GetDefaultFramebuffer() const override;

    private:
        static void __stdcall GlErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                    const GLchar* message, const GLvoid* userParam);

    private:
        std::unique_ptr<IStateManager> m_stateManager;
        std::unique_ptr<IResourceFactory> m_resourceFactory;
        std::unique_ptr<IRendererCapabilities> m_rendererCapabilities;
    };

} // namespace AT2::OpenGL

#endif
