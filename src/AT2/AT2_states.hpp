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

    //TODO: support separate blend functions, blend equations
    struct BlendMode
    {
        BlendFactor SourceFactor = BlendFactor::One;
        BlendFactor DestinationFactor = BlendFactor::Zero;
        bool Enabled = true;

        constexpr auto operator <=> (const BlendMode& ) const = default;
    };

    struct BlendColor
    {
        glm::vec4 Color {0.0f};
    };

    enum class ColorWriteFlags : std::uint8_t
    {
        None    = 0,
        Red     = 1 << 0,
        Green   = 1 << 2,
        Blue    = 1 << 3,
        Alpha   = 1 << 4,
        All     = Red | Green | Blue | Alpha,
    };
    using ColorMask = EnumFlags<ColorWriteFlags>;

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