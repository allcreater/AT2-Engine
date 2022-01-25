#ifndef GL_STATE_MANAGER_H
#define GL_STATE_MANAGER_H

#include <StateManager.h>
#include <lru_cache.h>

namespace AT2::OpenGL
{
class GlTexture;

class GlStateManager final : public StateManager
{
public:
    GlStateManager(IVisualizationSystem& visualizationSystem);

    void ApplyState(RenderState state) override;

	void Commit(const std::function<void(IUniformsWriter&)>& writer) override;

    std::optional<unsigned> GetActiveTextureIndex(std::shared_ptr<ITexture> texture) const noexcept override;

private:
    using TextureId = unsigned int;
    TextureId DoBind(std::shared_ptr<ITexture> texture);
    void DoBind(unsigned int index, const std::shared_ptr<IBuffer>& buffer);
    void DoBind(IShaderProgram& shader) override;
    void DoBind(IVertexArray& vertexArray) override;


private:
    std::vector<TextureId> m_freeTextureSlots;
    lru_cache<std::shared_ptr<ITexture>, TextureId> m_activeTextures;
};

};

#endif
