#ifndef AT2_STATE_MANAGER
#define AT2_STATE_MANAGER

#include "AT2.h"

namespace AT2
{
    class StateManager : public IStateManager
    {
    public:
        StateManager(IVisualizationSystem& renderer);

    public:
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

    protected:
        IVisualizationSystem& GetRenderer() const { return m_renderer; }

        virtual void DoBind(IShaderProgram& shader) = 0;
        virtual void DoBind(IVertexArray& vertexArray) = 0;

        IVisualizationSystem& m_renderer;

    private:

        std::shared_ptr<IShaderProgram> m_activeShader;
        std::shared_ptr<IVertexArray> m_activeVertexArray;

        std::optional<BufferDataType> m_activeIndexBufferType;
    };

} // namespace AT2

#endif
