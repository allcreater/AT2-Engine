#include "MtlStateManager.h"
#include "VertexArray.h"
#include "Renderer.h"
#include "ShaderProgram.h"
#include "Mappings.h"

using namespace AT2::Metal;

void MtlStateManager::ApplyState(RenderState state)
{
    std::visit(Utils::overloaded {
        [](const DepthState& state){
            //SetGlState(GL_DEPTH_TEST, state.TestEnabled);
            //glDepthMask(state.WriteEnabled);
            //glDepthFunc(AT2::Mappings::TranslateCompareFunction(state.CompareFunc));
        },
        [](const BlendMode& state){
            //SetGlState(GL_BLEND, state.Enabled);
            //if (!state.Enabled)
            //    return;

            //glBlendFunc(Mappings::TranslateBlendFactor(state.SourceFactor),
            //          Mappings::TranslateBlendFactor(state.DestinationFactor));
            //glBlendColor(state.BlendColor.r, state.BlendColor.g, state.BlendColor.b, state.BlendColor.a);
            

        },
        [this](const FaceCullMode& state){
            auto cullMode = Mappings::TranslateFaceCullMode(state);
            if (cullMode)
                GetRenderer().getFrameContext()->renderEncoder->setCullMode(*cullMode);
        },
        [this](const PolygonRasterizationMode& state){
            GetRenderer().getFrameContext()->renderEncoder->setTriangleFillMode(Mappings::TranslatePolygonRasterizationMode(state));
            
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

void MtlStateManager::DoBind(const ITexture& texture, unsigned index)
{
    
    //GetRenderer().getFrameContext()->renderEncoder->setVertexTexture(<#const MTL::Texture *texture#>, <#NS::UInteger index#>)
}

void MtlStateManager::DoBind(IFrameBuffer& frameBuffer)
{
    
}

void MtlStateManager::DoBind(IShaderProgram& shaderProgram)
{
    auto& mtlVertexArray = Utils::safe_dereference_cast<ShaderProgram&>(&shaderProgram);

    GetRenderer().UpdateStateParams([&](MTL::RenderPipelineDescriptor& params){
        auto* funcVS = mtlVertexArray.GetLibrary()->newFunction(NS::String::string("vertex_main", NS::ASCIIStringEncoding));
        auto* funcFS = mtlVertexArray.GetLibrary()->newFunction(NS::String::string("fragment_main", NS::ASCIIStringEncoding));
        params.setVertexFunction(funcVS);
        params.setFragmentFunction(funcFS);
    });
}

void MtlStateManager::DoBind(IVertexArray& vertexArray)
{
    auto& mtlVertexArray = Utils::safe_dereference_cast<VertexArray&>(&vertexArray);
 
    GetRenderer().UpdateStateParams([&](MTL::RenderPipelineDescriptor& params){
        params.setVertexDescriptor(mtlVertexArray.GetVertexDescriptor().get());
    });
}
