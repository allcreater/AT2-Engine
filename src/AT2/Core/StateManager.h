#ifndef AT2_STATE_MANAGER
#define AT2_STATE_MANAGER

#include "AT2.h"

#include <lru_cache.h>

namespace AT2
{
    class StateManager : public IStateManager
    {
    public:
        StateManager(IRenderer& renderer);

    public:
        void BindTextures(const TextureSet& _textures) override;
        void BindShader(const std::shared_ptr<IShaderProgram>& _shader) override;
        void BindVertexArray(const std::shared_ptr<IVertexArray>& _vertexArray) override;
        //TODO: BindIndexArray?

        //TextureSet& GetActiveTextures() const override;
        [[nodiscard]] std::shared_ptr<IShaderProgram> GetActiveShader() const override;
        [[nodiscard]] std::shared_ptr<IVertexArray> GetActiveVertexArray() const override;

        [[nodiscard]] std::optional<BufferDataType> GetIndexDataType() const noexcept override
        {
	        return m_activeIndexBufferType;
        }

        [[nodiscard]] std::optional<unsigned> GetActiveTextureIndex(std::shared_ptr<const ITexture> texture) const noexcept override;

    protected:
        IRenderer& GetRenderer() const { return m_renderer; }

        virtual void DoBind(const ITexture& texture, unsigned index) = 0;
        virtual void DoBind(IShaderProgram& shader) = 0;
        virtual void DoBind(IVertexArray& vertexArray) = 0;

    private:
        IRenderer& m_renderer;

        std::vector<unsigned int> m_freeTextureSlots;
        lru_cache<std::shared_ptr<const ITexture>, unsigned int> m_activeTextures;

        std::shared_ptr<IShaderProgram> m_activeShader;
        std::shared_ptr<IVertexArray> m_activeVertexArray;

        std::optional<BufferDataType> m_activeIndexBufferType;
    };

} // namespace AT2

#endif