#ifndef GL_STATE_MANAGER_H
#define GL_STATE_MANAGER_H

#include "AT2lowlevel.h"
#include <lru_cache.h>

namespace AT2::OpenGL41
{
class GlTexture;

class GlStateManager final : public IStateManager
{
public:
    GlStateManager(IVisualizationSystem& visualizationSystem);

    void BindVertexArray(const std::shared_ptr<IVertexArray>& vertexArray) override;

    void ApplyState(RenderState state) override;
    void ApplyPipelineState(const std::shared_ptr<IPipelineState>& state) override;

	void Commit(const std::function<void(IUniformsWriter&)>& writer) override;

    [[nodiscard]] std::shared_ptr<IShaderProgram> GetActiveShader() const override { return m_activeShader; }
    [[nodiscard]] std::shared_ptr<IVertexArray> GetActiveVertexArray() const override { return m_activeVertexArray; }

    [[nodiscard]] std::optional<BufferDataType> GetIndexDataType() const noexcept override
    {
        return m_activeIndexBufferType;
    }

    std::optional<unsigned> GetActiveTextureIndex(std::shared_ptr<ITexture> texture) const noexcept override;

// for internal usage
    using TextureId = unsigned int;
    TextureId DoBind(std::shared_ptr<ITexture> texture);

private:
    void DoBind(unsigned int index, const std::shared_ptr<IBuffer>& buffer);
    void DoBind(IShaderProgram& shader);
    void DoBind(IVertexArray& vertexArray);


private:
    std::shared_ptr<IShaderProgram> m_activeShader;
    std::shared_ptr<IVertexArray> m_activeVertexArray;

    std::optional<BufferDataType> m_activeIndexBufferType;

    std::vector<TextureId> m_freeTextureSlots;
    lru_cache<std::shared_ptr<ITexture>, TextureId> m_activeTextures;
};

};

#endif
