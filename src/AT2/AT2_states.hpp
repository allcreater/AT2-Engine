#ifndef AT2_STATES_H
#define AT2_STATES_H

#include "AT2_types.hpp"

namespace AT2
{
    enum class CompareFunction
    {
        Always,
        Equal,
        NotEqual,
        Greater,
        GreaterEqual,
        Less,
        LessEqual,
        Never
    };

    struct DepthState
    {
        CompareFunction CompareFunc {CompareFunction::Less};
        bool TestEnabled {false};
        bool WriteEnabled {true};
    };


    enum class BlendFactor
    {
        Zero,
        One,
        SourceColor,
        OneMinusSourceColor,
        DestinationColor,
        OneMinusDestinationColor,
        SourceAlpha,
        OneMinusSourceAlpha,
        DestinationAlpha,
        OneMinusDestinationAlpha,
        ConstantColor,
        OneMinusConstantColor,
        ConstantAlpha,
        OneMinusConstantAlpha
    };

    //TODO: support separate blend functions, blend equations, per-buffer blending
    struct BlendMode
    {
        BlendFactor SourceFactor = BlendFactor::One;
        BlendFactor DestinationFactor = BlendFactor::Zero;
        bool Enabled = true;
    };

    struct BlendColor
    {
        glm::vec4 Color {0.0f};
    };

    struct FaceCullMode
    {
        bool CullFront = false;
        bool CullBack = true;
    };

    enum class PolygonRasterizationMode
    {
        Point,
        Lines,
        Fill
    };

    enum class LineRasterizationMode
    {
        Normal,
        Smooth
    };

    // TODO: DepthBias, DepthStencilState, FaceWindig...
    using RenderState = std::variant<BlendColor, FaceCullMode, PolygonRasterizationMode, LineRasterizationMode>;


} // namespace AT2

#endif