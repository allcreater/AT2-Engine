#pragma once

#include "AT2lowlevel.h"

namespace AT2::Metal
{
class Renderer;

class ResourceFactory : public IResourceFactory
{
public:
    NON_COPYABLE_OR_MOVABLE(ResourceFactory)

    ResourceFactory(Renderer& renderer);
    ~ResourceFactory() override = default;

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
    Renderer& m_renderer;
    mutable std::vector<std::weak_ptr<IReloadable>> m_reloadableResourcesList;
};

}
