#include "MtlStateManager.h"
#include "VertexArray.h"
#include "Renderer.h"
#include "ShaderProgram.h"
#include "Mappings.h"
#include "Texture.h"

using namespace AT2::Metal;

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

Renderer& MtlStateManager::GetRenderer() const
{
    return static_cast<Renderer&>(StateManager::GetRenderer());
}

MtlPtr<MTL::RenderPipelineState> MtlStateManager::GetOrBuildState()
{
    if (m_currentState || m_stateInvalidated)
    {
        NS::Error* error;
        auto newState = Own(GetRenderer().getDevice()->newRenderPipelineState(m_buildingState.get(), &error));
        CheckErrors(error);
        
        m_currentState = std::move(newState);
        m_stateInvalidated = false;
    }
    
    return m_currentState;
}

void MtlStateManager::DoBind(ITexture& texture, unsigned index)
{
    
    if (!m_renderEncoder)
        return;
    
    m_renderEncoder->setVertexTexture(Utils::safe_dereference_cast<MtlTexture&>(&texture).getNativeHandle(), index);
}

void MtlStateManager::DoBind(IShaderProgram& shaderProgram)
{
    auto& mtlVertexArray = Utils::safe_dereference_cast<ShaderProgram&>(&shaderProgram);

    
    auto* funcVS = mtlVertexArray.GetLibrary()->newFunction(NS::String::string("vertex_main", NS::ASCIIStringEncoding));
    auto* funcFS = mtlVertexArray.GetLibrary()->newFunction(NS::String::string("fragment_main", NS::ASCIIStringEncoding));
    
    MTL::Argument* reflection;
    funcVS->newArgumentEncoder(0, &reflection);
    auto name = reflection->name()->cString(NS::ASCIIStringEncoding);
    auto* members = reflection->bufferStructType()->members();
    for (int i = 0; i < members->count(); ++i)
    {
        const auto* member = static_cast<MTL::StructMember*>(members->object(i));
        auto name = member->name()->cString(NS::UTF8StringEncoding);
        auto dataType = member->dataType();
    }
    
    m_buildingState->setVertexFunction(funcVS);
    m_buildingState->setFragmentFunction(funcFS);
    
    //TODO take actual attachment layout from active render stage?
    m_buildingState->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormatBGRA8Unorm);
    
    m_stateInvalidated = true;

}

void MtlStateManager::DoBind(IVertexArray& vertexArray)
{
    auto& mtlVertexArray = Utils::safe_dereference_cast<VertexArray&>(&vertexArray);
 
    m_buildingState->setVertexDescriptor(mtlVertexArray.GetVertexDescriptor().get());
    m_stateInvalidated = true;
    
    auto lastIndex = vertexArray.GetLastAttributeIndex();
    if (m_renderEncoder && lastIndex)
    {
        for (size_t index = 0; index <= *lastIndex; ++index)
        {
            if (auto buffer = mtlVertexArray.GetVertexBuffer(index))
                m_renderEncoder->setVertexBuffer(Utils::safe_dereference_cast<Buffer&>(buffer).getNativeHandle(), 0, index);
        }
    }
}
