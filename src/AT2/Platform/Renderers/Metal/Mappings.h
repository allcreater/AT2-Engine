#pragma once

#include "AT2lowlevel.h"

#include <AT2_textures.hpp>
#include <AT2_types.hpp>
#include <AT2_states.hpp>
#include <utils.hpp>

namespace AT2::Mappings
{
    constexpr MTL::SamplerAddressMode TranslateWrapMode(TextureWrapMode wrapMode)
    {
        switch (wrapMode)
        {
            case TextureWrapMode::ClampToEdge: return MTL::SamplerAddressModeClampToEdge;
            case TextureWrapMode::ClampToBorder: return MTL::SamplerAddressModeClampToBorderColor;
            case TextureWrapMode::MirroredRepeat: return MTL::SamplerAddressModeMirrorRepeat;
            case TextureWrapMode::Repeat: return MTL::SamplerAddressModeRepeat;
            case TextureWrapMode::MirrorClampToEdge: return MTL::SamplerAddressModeMirrorClampToEdge;
            //SamplerAddressModeClampToZero is unsuppported
        }

        throw AT2Exception("Unsupported TextureWrapMode");
    }
/*
    constexpr GLenum TranslateExternalFormat(TextureLayout layout)
    {
        switch (layout)
        {
        case TextureLayout::Red: return GL_RED;
        case TextureLayout::RG: return GL_RG;
        case TextureLayout::RGB: return GL_RGB;
        case TextureLayout::BGR: return GL_BGR;
        case TextureLayout::BGRA: return GL_BGRA;
        case TextureLayout::RGBA: return GL_RGBA;
        case TextureLayout::DepthComponent: return GL_DEPTH_COMPONENT;
        case TextureLayout::StencilIndex: return GL_STENCIL_INDEX;
        }

        assert(false);
        return 0;
    }

    constexpr GLenum TranslateExternalType(BufferDataType type)
    {
        switch (type)
        {
        case BufferDataType::Byte: return GL_BYTE;
        case BufferDataType::UByte: return GL_UNSIGNED_BYTE;
        case BufferDataType::Short: return GL_SHORT;
        case BufferDataType::UShort: return GL_UNSIGNED_SHORT;
        case BufferDataType::Int: return GL_INT;
        case BufferDataType::UInt: return GL_UNSIGNED_INT;
        case BufferDataType::HalfFloat: return GL_HALF_FLOAT;
        case BufferDataType::Float: return GL_FLOAT;
        case BufferDataType::Double: return GL_DOUBLE;
        case BufferDataType::Fixed: return GL_FIXED;
        }

        assert(false);
        return 0;
    }

    constexpr GLenum TranslateBufferType(VertexBufferType bufferType)
    {
        switch (bufferType)
        {
        case VertexBufferType::ArrayBuffer: return GL_ARRAY_BUFFER;
        case VertexBufferType::IndexBuffer: return GL_ELEMENT_ARRAY_BUFFER;
        case VertexBufferType::UniformBuffer: return GL_UNIFORM_BUFFER;
        }

        assert(false);
        return 0;
    }

    constexpr GLenum TranslateBufferUsage(BufferUsage bufferUsage)
    {
        switch (bufferUsage)
        {
        case BufferUsage::Read: return GL_READ_ONLY;
        case BufferUsage::ReadWrite: return GL_READ_WRITE;
        case BufferUsage::Write: return GL_WRITE_ONLY;
        }

        assert(false);
        return 0;
    }
 */

    constexpr MTL::TextureType TranslateTextureTarget(const Texture& texture)
    {
        return std::visit(
            Utils::overloaded {
                [](const Texture1D&) { return MTL::TextureType1D; },
                [](const Texture1DArray&) { return MTL::TextureType1DArray; },
                [](const Texture2DArray&) { return MTL::TextureType2DArray; },
                [](const Texture2D&) { return MTL::TextureType2D; },
                [](const Texture2DMultisample&) { return MTL::TextureType2DMultisample; },
                [](const Texture2DRectangle&) { return MTL::TextureType2D; },
                [](const Texture2DMultisampleArray&) { return MTL::TextureType2DMultisampleArray; },
                [](const TextureCube&) { return MTL::TextureTypeCube; },
                [](const TextureCubeArray&) { return MTL::TextureTypeCubeArray; },
                [](const Texture3D&) { return MTL::TextureType3D; }
                //MTL::TextureTypeTextureBuffer
            }, texture);
    }

    inline MTL::PrimitiveType TranslatePrimitiveType(const Primitives::Primitive& primitive)
    {
        using namespace Primitives;

        return std::visit(
            Utils::overloaded {
                [](const Points&) { return MTL::PrimitiveTypePoint; },
                [](const LineStrip&) { return MTL::PrimitiveTypeLineStrip; },
//              [](const LineLoop&) { return GL_LINE_LOOP; },
                [](const Lines&) { return MTL::PrimitiveTypeLine; },
//              [](const LineStripAdjacency&) { return GL_LINE_STRIP_ADJACENCY; },
//              [](const LinesAdjacency&) { return GL_LINES_ADJACENCY; },
                [](const TriangleStrip&) { return MTL::PrimitiveTypeTriangleStrip; },
//              [](const TriangleFan&) { return GL_TRIANGLE_FAN; },
                [](const Triangles&) { return MTL::PrimitiveTypeTriangle; },
//              [](const TriangleStripAdjacency&) { return GL_TRIANGLE_STRIP_ADJACENCY; },
//              [](const TrianglesAdjacency&) { return GL_TRIANGLES_ADJACENCY; },
//              [](const Patches&) { return GL_PATCHES; }},
                [](const auto&) -> MTL::PrimitiveType { throw AT2Exception("This primitive type is not supported by Metal"); }
            }, primitive);
    }
/*
    constexpr GLenum TranslateCompareFunction(CompareFunction function)
    {
        switch (function)
        {
        case CompareFunction::Always: return GL_ALWAYS;
        case CompareFunction::Equal: return GL_EQUAL;
        case CompareFunction::NotEqual: return GL_NOTEQUAL;
        case CompareFunction::Greater: return GL_GREATER;
        case CompareFunction::GreaterEqual: return GL_GEQUAL;
        case CompareFunction::Less: return GL_LESS;
        case CompareFunction::LessEqual: return GL_LEQUAL;
        case CompareFunction::Never: return GL_NEVER;
        }

        assert(false);
        return 0;
    }

    constexpr GLenum TranslateBlendFactor(BlendFactor factor)
    {
        switch (factor)
        {
        case BlendFactor::Zero: return GL_ZERO;
        case BlendFactor::One: return GL_ONE;
        case BlendFactor::SourceColor: return GL_SRC_COLOR;
        case BlendFactor::OneMinusSourceColor: return GL_ONE_MINUS_SRC_COLOR;
        case BlendFactor::DestinationColor: return GL_DST_COLOR;
        case BlendFactor::OneMinusDestinationColor: return GL_ONE_MINUS_DST_COLOR;
        case BlendFactor::SourceAlpha: return GL_SRC_ALPHA;
        case BlendFactor::OneMinusSourceAlpha: return GL_ONE_MINUS_SRC_ALPHA;
        case BlendFactor::DestinationAlpha: return GL_DST_ALPHA;
        case BlendFactor::OneMinusDestinationAlpha: return GL_ONE_MINUS_DST_ALPHA;
        case BlendFactor::ConstantColor: return GL_CONSTANT_COLOR;
        case BlendFactor::OneMinusConstantColor: return GL_ONE_MINUS_CONSTANT_COLOR;
        case BlendFactor::ConstantAlpha: return GL_CONSTANT_ALPHA;
        case BlendFactor::OneMinusConstantAlpha: return GL_ONE_MINUS_CONSTANT_ALPHA;
        }

        assert(false);
        return 0;
    }
*/
    constexpr std::optional<MTL::CullMode> TranslateFaceCullMode(FaceCullMode mode)
    {
        switch ((mode.CullFront ? 0b10 : 0) | (mode.CullBack ? 0b01 : 0))
        {
        case 0b00: return MTL::CullModeNone;
        case 0b01: return MTL::CullModeBack;
        case 0b10: return MTL::CullModeFront;
        case 0b11: return std::nullopt;
        }

        throw AT2Exception("Unsupported FaceCullMode");
    }

    constexpr MTL::TriangleFillMode TranslatePolygonRasterizationMode(PolygonRasterizationMode mode)
    {
        switch (mode)
        {
            //case PolygonRasterizationMode::Point:
            case PolygonRasterizationMode::Lines: return MTL::TriangleFillModeLines;
            case PolygonRasterizationMode::Fill: return MTL::TriangleFillModeFill;
        }

        throw AT2Exception("Unsupported PolygonRasterizationMode");
    }

    constexpr MTL::SamplerMipFilter TranslateTextureMipMode(MipmapSamplingMode mipmapMode)
    {
        switch (mipmapMode)
        {
            case MipmapSamplingMode::Manual:    return MTL::SamplerMipFilterNotMipmapped;
            case MipmapSamplingMode::Nearest:   return MTL::SamplerMipFilterNearest;
            case MipmapSamplingMode::Linear:    return MTL::SamplerMipFilterLinear;
        }
    }

    constexpr MTL::SamplerMinMagFilter TranslateTextureSamplingMode(TextureSamplingMode samplingMode)
    {
        switch (samplingMode)
        {
            case TextureSamplingMode::Nearest:  return MTL::SamplerMinMagFilterNearest;
            case TextureSamplingMode::Linear:   return MTL::SamplerMinMagFilterLinear;
        }

        throw AT2Exception("Unsupported TextureSamplingMode");
    }
/*
    constexpr GLboolean TranslateBool(bool value)
    {
        return value ? GL_TRUE : GL_FALSE;
    }
 */

} // namespace AT2::Mappings
