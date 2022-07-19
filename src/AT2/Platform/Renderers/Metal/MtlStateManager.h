#pragma once

#include "AT2lowlevel.h"

namespace AT2::Metal
{

class Buffer;
class MtlTexture;
class Renderer;
class ShaderProgram;
class VertexArray;

class MtlStateManager : public IStateManager, public IRenderer
{
public:
    MtlStateManager(Renderer& renderer, MTL::RenderCommandEncoder* encoder);
    ~MtlStateManager() override = default;
    
    void Commit(const std::function<void(IUniformsWriter&)>& writeComand) override;
    
//IStateManager interface
    void BindShader(const std::shared_ptr<IShaderProgram>& shader) override;
    void BindVertexArray(const std::shared_ptr<IVertexArray>& vertexArray) override;

    void ApplyState(RenderState state) override;

    [[nodiscard]] std::shared_ptr<IShaderProgram> GetActiveShader() const override;
    [[nodiscard]] std::shared_ptr<IVertexArray> GetActiveVertexArray() const override;

    [[nodiscard]] std::optional<BufferDataType> GetIndexDataType() const noexcept override;

	[[nodiscard]] std::optional<unsigned int> GetActiveTextureIndex(std::shared_ptr<ITexture> texture) const noexcept override { return std::nullopt; }
    
//IRenderer interface
    void Draw(Primitives::Primitive type, size_t first, long int count, int numInstances, int baseVertex) override;

    void SetViewport(const AABB2d& viewport) override;
    void SetScissorWindow(const AABB2d& viewport) override;
    [[nodiscard]] IStateManager& GetStateManager() override { return *this; }

    IVisualizationSystem& GetVisualizationSystem() override;
    
private:
    void BindBuffer(std::shared_ptr<IBuffer>, ResourceBindingPoint bindingPoint);
    void BindTexture(std::shared_ptr<MtlTexture>, ResourceBindingPoint bindingPoint);
    
    MtlPtr<MTL::RenderPipelineState> GetOrBuildState();
    
private:
    Renderer& m_renderer;
    MTL::RenderCommandEncoder* m_renderEncoder;
    
    std::shared_ptr<ShaderProgram> m_activeShader;
    std::shared_ptr<VertexArray> m_activeVertexArray;
    
    MtlPtr<MTL::RenderPipelineDescriptor> m_buildingState = ConstructMetalObject<MTL::RenderPipelineDescriptor>();
    MtlPtr<MTL::DepthStencilDescriptor> m_buildingDepthStencilState;
    MtlPtr<MTL::RenderPipelineState> m_currentState;
    
    bool m_stateInvalidated = true;
    bool m_buildingDepthStencilStateInvalidated = true;
};

};
