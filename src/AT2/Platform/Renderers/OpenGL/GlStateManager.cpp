#include "GlStateManager.h"
#include "AT2lowlevel.h"
#include "Mappings.h"

using namespace AT2;

void OpenGL::GlStateManager::ApplyState(RenderState state)
{
    //TODO: remember current state or even state stack?
    std::visit(Utils::overloaded {
        [](const DepthState& state){
            SetGlState(GL_DEPTH_TEST, state.TestEnabled);
            glDepthMask(state.WriteEnabled);
            glDepthFunc(AT2::Mappings::TranslateCompareFunction(state.CompareFunc));
        },
        [](const BlendMode& state){
            SetGlState(GL_BLEND, state.Enabled);
            if (!state.Enabled)
                return;

            glBlendFunc(Mappings::TranslateBlendFactor(state.SourceFactor),
                      Mappings::TranslateBlendFactor(state.DestinationFactor));
            glBlendColor(state.BlendColor.r, state.BlendColor.g, state.BlendColor.b, state.BlendColor.a);
        },
        [](const FaceCullMode& state){
            const auto mode = Mappings::TranslateFaceCullMode(state);
            SetGlState(GL_CULL_FACE, mode != 0);
            glCullFace(mode);
        },
		[](const PolygonRasterizationMode& state){
            glPolygonMode(GL_FRONT_AND_BACK, Mappings::TranslatePolygonRasterizationMode(state));
        },
		[](const LineRasterizationMode& state){
			SetGlState(GL_LINE_SMOOTH, state == LineRasterizationMode::Smooth);
		}
    }, state);
}
