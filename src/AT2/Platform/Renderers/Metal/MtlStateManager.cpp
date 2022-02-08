#include "MtlStateManager.h"
#include "VertexArray.h"
#include "Renderer.h"
#include "ShaderProgram.h"
#include <DataLayout/StructuredBuffer.h>
#include "Mappings.h"
#include "Texture.h"

using namespace AT2;
using namespace AT2::Metal;

MtlStateManager::MtlStateManager(Renderer& renderer, MTL::RenderCommandEncoder* encoder)
: m_renderer{renderer}
, m_renderEncoder{encoder}
{
    m_renderEncoder->setFrontFacingWinding(MTL::WindingCounterClockwise); //as in OpenGLMtlStateManager
}

void MtlStateManager::ApplyState(RenderState state)
{
    std::visit(Utils::overloaded {
        [this](const DepthState& state){
            m_buildingDepthStencilState->setDepthWriteEnabled(state.WriteEnabled);
            m_buildingDepthStencilState->setDepthCompareFunction( state.TestEnabled ? Mappings::TranslateCompareFunction(state.CompareFunc) : MTL::CompareFunctionAlways);
            
            m_buildingDepthStencilStateInvalidated = true;
        },
        [this](const BlendMode& state){
            auto* attachment = m_buildingState->colorAttachments()->object(0);
            
            attachment->setBlendingEnabled(state.Enabled);
            attachment->setSourceRGBBlendFactor(Mappings::TranslateBlendFactor(state.SourceFactor));
            attachment->setSourceAlphaBlendFactor(Mappings::TranslateBlendFactor(state.SourceFactor));
            attachment->setDestinationRGBBlendFactor(Mappings::TranslateBlendFactor(state.DestinationFactor));
            attachment->setDestinationAlphaBlendFactor(Mappings::TranslateBlendFactor(state.DestinationFactor));
            
            m_stateInvalidated = true;
            
            if (m_renderEncoder)
                m_renderEncoder->setBlendColorRed(state.BlendColor.r, state.BlendColor.g, state.BlendColor.b, state.BlendColor.a);
        },
        [this](const FaceCullMode& state){
            auto cullMode = Mappings::TranslateFaceCullMode(state);
            
            if (!m_renderEncoder)
                return;
            
            if (cullMode)
                m_renderEncoder->setCullMode(*cullMode);
            
        },
        [this](const PolygonRasterizationMode& state){
            if (m_renderEncoder)
                m_renderEncoder->setTriangleFillMode(Mappings::TranslatePolygonRasterizationMode(state));
            
        },
        [](const LineRasterizationMode& state){
            //SetGlState(GL_LINE_SMOOTH, state == LineRasterizationMode::Smooth);
        }
    }, state);
}

[[nodiscard]] std::shared_ptr<IShaderProgram> MtlStateManager::GetActiveShader() const
{
    return m_activeShader;
}
[[nodiscard]] std::shared_ptr<IVertexArray> MtlStateManager::GetActiveVertexArray() const
{
    return m_activeVertexArray;
}

[[nodiscard]] std::optional<BufferDataType> MtlStateManager::GetIndexDataType() const noexcept
{
    return m_activeVertexArray ? m_activeVertexArray->GetIndexBufferType() : std::nullopt;
}

void MtlStateManager::Draw(Primitives::Primitive type, size_t first, long int count, int numInstances, int baseVertex)
{
    auto state = GetOrBuildState();
    m_renderEncoder->setRenderPipelineState(state.get());
    
    const auto platformPrimitiveType = Mappings::TranslatePrimitiveType(type);
    if (auto indexBufferDataType = m_activeVertexArray->GetIndexBufferType())
    {
        auto& mtlIndexBuffer = Utils::safe_dereference_cast<Buffer>(m_activeVertexArray->GetIndexBuffer());
        m_renderEncoder->drawIndexedPrimitives(platformPrimitiveType, count, Mappings::TranslateIndexBufferType(*indexBufferDataType), mtlIndexBuffer.getNativeHandle(), 0, numInstances, baseVertex, 0);
    }
    else
        m_renderEncoder->drawPrimitives(platformPrimitiveType, first, count, numInstances);
}

void MtlStateManager::SetViewport(const AABB2d& viewport)
{
    m_renderEncoder->setViewport(MTL::Viewport{viewport.MinBound.x, viewport.MinBound.y, viewport.GetWidth(), viewport.GetHeight(), 0.0f, 1.0f});
}

void MtlStateManager::SetScissorWindow(const AABB2d& window)
{
    MTL::ScissorRect rect{
        window.MinBound.x >= 0.0f ? static_cast<NS::UInteger>(window.MinBound.x) : 0,
        window.MinBound.y >= 0.0f ? static_cast<NS::UInteger>(window.MinBound.y) : 0,
        static_cast<NS::UInteger>(window.GetWidth()),
        static_cast<NS::UInteger>(window.GetHeight())
        
    };
    
    m_renderEncoder->setScissorRect(rect);
}

IVisualizationSystem& MtlStateManager::GetVisualizationSystem()
{
    return m_renderer;
}


MtlPtr<MTL::RenderPipelineState> MtlStateManager::GetOrBuildState()
{
    if (m_currentState || m_stateInvalidated)
    {
        NS::Error* error;
        MTL::RenderPipelineReflection* reflection;
        auto newState = Own(m_renderer.getDevice()->newRenderPipelineState(m_buildingState.get(),MTL::PipelineOptionArgumentInfo | MTL::PipelineOptionBufferTypeInfo, &reflection, &error));
        
        CheckErrors(error);
        
        if (m_activeShader)
        {
            m_activeShader->OnStateCreated(reflection);
            if (auto uniformBuffer = m_activeShader->GetDefaultUniformBlock())
                SetUniform(DefaultUniformBlockName, uniformBuffer->GetBuffer());
        }
        
        m_currentState = std::move(newState);
        m_stateInvalidated = false;
    }
    
    return m_currentState;
}


void MtlStateManager::Commit(const std::function<void(IUniformsWriter&)>& writeCommand)
{
    class ImmediateUniformWriter : public IUniformsWriter
    {
    public:
        explicit ImmediateUniformWriter(MtlStateManager& stateManager, IUniformsWriter* defaultStorageWriter)
        : m_stateManager {stateManager}
        , m_activeProgram {Utils::safe_dereference_cast<ShaderProgram&>(m_stateManager.GetActiveShader()) }
        , m_defaultUniformStorageWriter {defaultStorageWriter}
        {}
        
        void Write(std::string_view name, Uniform value) override
        {
            if (m_defaultUniformStorageWriter)
                m_defaultUniformStorageWriter->Write(name, value);
        }
        
        void Write(std::string_view name, UniformArray value) override
        {
            if (m_defaultUniformStorageWriter)
                m_defaultUniformStorageWriter->Write(name, value);
        }
        
        void Write(std::string_view name, std::shared_ptr<ITexture> texture) override
        {
            auto mtlTexture = std::dynamic_pointer_cast<MtlTexture>(texture);
            assert(mtlTexture);
            
            m_stateManager.m_activeShader->GetIntrospection()->FindTexture(name, [&](const Introspection::ArgumentInfo& paramInfo){
                m_stateManager.BindTexture(mtlTexture, paramInfo);
            });
        }

        void Write(std::string_view name, std::shared_ptr<IBuffer> buffer) override
        {
            auto mtlBuffer = std::dynamic_pointer_cast<Buffer>(buffer);
            assert(mtlBuffer);
            
            m_stateManager.m_activeShader->GetIntrospection()->FindBuffer(name, [&](const Introspection::BufferInfo& paramInfo){
                m_stateManager.BindBuffer(mtlBuffer, paramInfo);
            });
        }

    private:
        MtlStateManager& m_stateManager;
        ShaderProgram& m_activeProgram;
        IUniformsWriter* m_defaultUniformStorageWriter;
    };

    if (!m_activeShader || !m_activeShader->GetIntrospection())
        return;
    
    auto doWrite = [this, &writeCommand](IUniformsWriter* defaultStorageWriter)
    {
        ImmediateUniformWriter writer {*this, defaultStorageWriter};
        writeCommand(writer);
    };
    
    // if default buffer is present, "usual" parameters will be redirected to it
    if (auto defaultUniformBlock = m_activeShader->GetDefaultUniformBlock())
        defaultUniformBlock->Commit([doWrite](IUniformsWriter& writer){ doWrite(&writer); });
    else
        doWrite(nullptr);
    
}

void MtlStateManager::BindShader(const std::shared_ptr<IShaderProgram>& shaderProgram)
{
    m_activeShader = std::dynamic_pointer_cast<ShaderProgram>(shaderProgram);
    
    m_activeShader->Apply(*m_buildingState);
    
    //TODO take actual attachment layout from active render stage?
    m_buildingState->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormatBGRA8Unorm);
    m_stateInvalidated = true;
}

void MtlStateManager::BindVertexArray(const std::shared_ptr<IVertexArray>& vertexArray)
{
    m_activeVertexArray = std::dynamic_pointer_cast<VertexArray>(vertexArray);
    
    m_buildingState->setVertexDescriptor(m_activeVertexArray->GetVertexDescriptor().get());
    m_stateInvalidated = true;
    
    auto lastIndex = m_activeVertexArray->GetLastAttributeIndex();
    if (m_renderEncoder && lastIndex)
    {
        for (size_t index = 0; index <= *lastIndex; ++index)
        {
            if (auto buffer = m_activeVertexArray->GetVertexBuffer(index))
                m_renderEncoder->setVertexBuffer(Utils::safe_dereference_cast<Buffer&>(buffer).getNativeHandle(), 0, index);
        }
    }
}

//TODO: track active textures and buffers, are we StateManager or not?
void MtlStateManager::BindBuffer(std::shared_ptr<Buffer> buffer, ResourceBindingPoint bindingPoint)
{
    switch (bindingPoint.Target)
    {
        case AttachmentTarget::Vertex:
            m_renderEncoder->setVertexBuffer(buffer->getNativeHandle(), 0, bindingPoint.Index);
            break;
        case AttachmentTarget::Fragment:
            m_renderEncoder->setFragmentBuffer(buffer->getNativeHandle(), 0, bindingPoint.Index);
            break;
        case AttachmentTarget::Tile:
            m_renderEncoder->setTileBuffer(buffer->getNativeHandle(), 0, bindingPoint.Index);
            break;
    }
}

void MtlStateManager::BindTexture(std::shared_ptr<MtlTexture> texture, ResourceBindingPoint bindingPoint)
{
    switch (bindingPoint.Target)
    {
        case AttachmentTarget::Vertex:
            m_renderEncoder->setVertexTexture(texture->getNativeHandle(), bindingPoint.Index);
            break;
        case AttachmentTarget::Fragment:
            m_renderEncoder->setFragmentTexture(texture->getNativeHandle(), bindingPoint.Index);
            break;
        case AttachmentTarget::Tile:
            m_renderEncoder->setTileTexture(texture->getNativeHandle(), bindingPoint.Index);
            break;
    }
}
