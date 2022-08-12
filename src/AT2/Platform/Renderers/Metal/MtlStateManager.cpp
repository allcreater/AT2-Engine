#include "MtlStateManager.h"
#include "ArrayBuffer.h"
#include "VertexArray.h"
#include "Renderer.h"
#include "ShaderProgram.h"
#include <DataLayout/StructuredBuffer.h>
#include "Mappings.h"
#include "Texture.h"
#include "PipelineState.h"

#include <iomanip>

using namespace AT2;
using namespace AT2::Metal;

namespace
{

void BindNativeBuffer(MTL::RenderCommandEncoder& renderEncoder, Buffer& buffer, unsigned int offset, ResourceBindingPoint bindingPoint)
{
    auto handle = buffer.getNativeHandle();
    switch (bindingPoint.Target)
    {
        case AttachmentTarget::Vertex:
            renderEncoder.setVertexBuffer(handle, offset, bindingPoint.Index);
            break;
        case AttachmentTarget::Fragment:
            renderEncoder.setFragmentBuffer(handle, offset, bindingPoint.Index);
            break;
        case AttachmentTarget::Tile:
            renderEncoder.setTileBuffer(handle, offset, bindingPoint.Index);
            break;
    }
}

void BindArray(MTL::RenderCommandEncoder& renderEncoder, AT2::IBuffer& buffer, ResourceBindingPoint bindingPoint)
{
    auto range = buffer.Map(BufferOperationFlags::Read);
    
    switch (bindingPoint.Target)
    {
        case AttachmentTarget::Vertex:
            renderEncoder.setVertexBytes(range.data(), range.size(), bindingPoint.Index);
            break;
        case AttachmentTarget::Fragment:
            renderEncoder.setFragmentBytes(range.data(), range.size(), bindingPoint.Index);
            break;
        case AttachmentTarget::Tile:
            renderEncoder.setTileBytes(range.data(), range.size(), bindingPoint.Index);
            break;
    }

    buffer.Unmap();
}

void BindTexture(MTL::RenderCommandEncoder& renderEncoder, MtlTexture& texture, ResourceBindingPoint bindingPoint)
{
    auto handle = texture.getNativeHandle();
    switch (bindingPoint.Target)
    {
        case AttachmentTarget::Vertex:
            renderEncoder.setVertexTexture(handle, bindingPoint.Index);
            break;
        case AttachmentTarget::Fragment:
            renderEncoder.setFragmentTexture(handle, bindingPoint.Index);
            break;
        case AttachmentTarget::Tile:
            renderEncoder.setTileTexture(handle, bindingPoint.Index);
            break;
    }
}

}


MtlStateManager::MtlStateManager(Renderer& renderer, MTL::RenderCommandEncoder* encoder)
: m_renderer{renderer}
, m_renderEncoder{encoder}
{
    m_renderEncoder->setFrontFacingWinding(MTL::WindingCounterClockwise); //as in OpenGLMtlStateManager
}

void MtlStateManager::ApplyState(RenderState state)
{
    if (!m_renderEncoder)
        return;

    std::visit(Utils::overloaded {
        [this](const BlendColor& state){
            m_renderEncoder->setBlendColorRed(state.Color.r, state.Color.g, state.Color.b, state.Color.a);
        },
        [this](const FaceCullMode& state){
            auto cullMode = Mappings::TranslateFaceCullMode(state);
            if (cullMode)
                m_renderEncoder->setCullMode(*cullMode);
        },
        [this](const PolygonRasterizationMode& state){
            m_renderEncoder->setTriangleFillMode(Mappings::TranslatePolygonRasterizationMode(state));
        },
        [](const LineRasterizationMode& state){
            //Seems it's not supported
        }
    }, state);
}

std::shared_ptr<IShaderProgram> MtlStateManager::GetActiveShader() const
{
    return m_activeShader;
}
std::shared_ptr<IVertexArray> MtlStateManager::GetActiveVertexArray() const
{
    return m_activeVertexArray;
}

std::optional<BufferDataType> MtlStateManager::GetIndexDataType() const noexcept
{
    return m_activeVertexArray ? m_activeVertexArray->GetIndexBufferType() : std::nullopt;
}

void MtlStateManager::Draw(Primitives::Primitive type, size_t first, long int count, int numInstances, int baseVertex)
{
    //TODO: crutch
    if (auto uniformBuffer = m_activeShader->GetDefaultUniformBlock())
    {
        m_activeShader->GetIntrospection()->FindBuffer(DefaultUniformBlockName, [&](const Introspection::BufferInfo& paramInfo){
            BindBuffer(uniformBuffer->GetBuffer(), paramInfo);
        });
    }
    
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
            m_stateManager.m_activeShader->GetIntrospection()->FindBuffer(name, [&](const Introspection::BufferInfo& paramInfo){
                m_stateManager.BindBuffer(buffer, paramInfo);
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

void MtlStateManager::ApplyPipelineState(const std::shared_ptr<IPipelineState>& state)
{
    auto& mtlPipelineState = Utils::safe_dereference_cast<PipelineState&>(state);
    m_activeShader = mtlPipelineState.GetShaderProgram();
    
    mtlPipelineState.Apply(*m_renderEncoder);
}

void MtlStateManager::BindVertexArray(const std::shared_ptr<IVertexArray>& vertexArray)
{
    m_activeVertexArray = std::dynamic_pointer_cast<VertexArray>(vertexArray);
    m_activeVertexArray->Apply(*m_renderEncoder);
}

//TODO: track active textures and buffers, are we StateManager or not?
void MtlStateManager::BindBuffer(std::shared_ptr<IBuffer> buffer, ResourceBindingPoint bindingPoint)
{
    assert(buffer);
    auto&& bufferType = typeid(*buffer);
    
    if (bufferType == typeid(Buffer))
        BindNativeBuffer(*m_renderEncoder, static_cast<Buffer&>(*buffer), 0, bindingPoint);
    else if (bufferType == typeid(ArrayBuffer))
        BindArray(*m_renderEncoder, *buffer, bindingPoint);
    else
        assert(false);
}

void MtlStateManager::BindTexture(std::shared_ptr<MtlTexture> texture, ResourceBindingPoint bindingPoint)
{
    assert(texture);
    ::BindTexture(*m_renderEncoder, *texture, bindingPoint);
}
