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
        CompareFunction CompareFunction {CompareFunction::Less};
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
    struct BlendingState
    {
        BlendFactor SourceFactor;
        BlendFactor DestinationFactor;
        glm::vec4 BlendColor {0.0f};
    };

    using RenderState = std::variant<DepthState, BlendingState>;


} // namespace AT2

#endif