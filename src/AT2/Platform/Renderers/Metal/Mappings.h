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

    constexpr inline std::array texture_formats_lookup {
        std::pair{TextureFormat::R8Snorm,               MTL::PixelFormatR8Snorm},
        std::pair{TextureFormat::R8Unorm,               MTL::PixelFormatR8Unorm},
        std::pair{TextureFormat::R16Snorm,              MTL::PixelFormatR16Snorm},
        std::pair{TextureFormat::R16Unorm,              MTL::PixelFormatR16Unorm},
        std::pair{TextureFormat::R16Sint,               MTL::PixelFormatR16Sint},
        std::pair{TextureFormat::R16Uint,               MTL::PixelFormatR16Uint},
        std::pair{TextureFormat::R32Sint,               MTL::PixelFormatR32Sint},
        std::pair{TextureFormat::R32Uint,               MTL::PixelFormatR32Uint},
        std::pair{TextureFormat::R16Float,              MTL::PixelFormatR16Float},
        std::pair{TextureFormat::R32Float,              MTL::PixelFormatR32Float},
        std::pair{TextureFormat::RG8Snorm,              MTL::PixelFormatRG8Snorm},
        std::pair{TextureFormat::RG8Unorm,              MTL::PixelFormatRG8Unorm},
        std::pair{TextureFormat::RG16Snorm,             MTL::PixelFormatRG16Snorm},
        std::pair{TextureFormat::RG16Unorm,             MTL::PixelFormatRG16Unorm},
        std::pair{TextureFormat::RG16Sint,              MTL::PixelFormatRG16Sint},
        std::pair{TextureFormat::RG16Uint,              MTL::PixelFormatRG16Uint},
        std::pair{TextureFormat::RG32Sint,              MTL::PixelFormatRG32Sint},
        std::pair{TextureFormat::RG32Uint,              MTL::PixelFormatRG32Uint},
        std::pair{TextureFormat::RG16Float,             MTL::PixelFormatRG16Float},
        std::pair{TextureFormat::RG32Float,             MTL::PixelFormatRG32Float},
        std::pair{TextureFormat::RGBA8Snorm,            MTL::PixelFormatRGBA8Snorm},
        std::pair{TextureFormat::RGBA8Unorm,            MTL::PixelFormatRGBA8Unorm},
        std::pair{TextureFormat::RGBA8Unorm_sRGB,       MTL::PixelFormatRGBA8Unorm_sRGB},
        std::pair{TextureFormat::BGRA8Unorm,            MTL::PixelFormatBGRA8Unorm},
        std::pair{TextureFormat::BGRA8Unorm_sRGB,       MTL::PixelFormatBGRA8Unorm_sRGB},
        std::pair{TextureFormat::RGBA16Snorm,           MTL::PixelFormatRGBA16Snorm},
        std::pair{TextureFormat::RGBA16Unorm,           MTL::PixelFormatRGBA16Unorm},
        std::pair{TextureFormat::RGBA8Sint,             MTL::PixelFormatRGBA8Sint},
        std::pair{TextureFormat::RGBA8Uint,             MTL::PixelFormatRGBA8Uint},
        std::pair{TextureFormat::RGBA16Sint,            MTL::PixelFormatRGBA16Sint},
        std::pair{TextureFormat::RGBA16Uint,            MTL::PixelFormatRGBA16Uint},
        std::pair{TextureFormat::RGBA32Sint,            MTL::PixelFormatRGBA32Sint},
        std::pair{TextureFormat::RGBA32Uint,            MTL::PixelFormatRGBA32Uint},
        std::pair{TextureFormat::RGBA16Float,           MTL::PixelFormatRGBA16Float},
        std::pair{TextureFormat::RGBA32Float,           MTL::PixelFormatRGBA32Float},
        std::pair{TextureFormat::Depth16Unorm,          MTL::PixelFormatDepth16Unorm},
        std::pair{TextureFormat::Depth32Float,          MTL::PixelFormatDepth32Float},
        std::pair{TextureFormat::Depth24Unorm_Stencil8, MTL::PixelFormatDepth24Unorm_Stencil8},
        std::pair{TextureFormat::Depth32Float_Stencil8, MTL::PixelFormatDepth32Float_Stencil8},
    };

    constexpr MTL::PixelFormat TranslateTextureFormat(TextureFormat format)
    {
        static_assert(static_cast<size_t>(TextureFormat::R8Snorm) == 0);
        static_assert(static_cast<size_t>(TextureFormat::Depth32Float_Stencil8) + 1 == texture_formats_lookup.size());
        
        if (static_cast<size_t>(format) >= texture_formats_lookup.size())
            throw AT2Exception("Unsupported external format ChannelsLayout");

        return texture_formats_lookup[static_cast<size_t>(format)].second;
    }

    constexpr TextureFormat TranslateTextureFormatBack(MTL::PixelFormat format)
    {
        auto it = std::find_if(texture_formats_lookup.begin(), texture_formats_lookup.end(), [format](const auto& pair){ return pair.second == format; });
        if (it == texture_formats_lookup.end())
            throw AT2Exception("This MTL::PixelFormat is not supported by AT2");

        return it->first;
    }

    constexpr MTL::VertexFormat TranslateVertexFormat(BufferDataType type, uint8_t count, bool normalized)
    {
        switch (count)
        {
            case 1:
                switch (type)
                {
                    case BufferDataType::Byte: return normalized ? MTL::VertexFormatCharNormalized : MTL::VertexFormatChar;
                    case BufferDataType::UByte: return normalized ? MTL::VertexFormatUCharNormalized : MTL::VertexFormatUChar;
                    case BufferDataType::Short: return normalized ? MTL::VertexFormatShortNormalized : MTL::VertexFormatShort;
                    case BufferDataType::UShort: return normalized ? MTL::VertexFormatUShortNormalized : MTL::VertexFormatUShort;
                    case BufferDataType::Int: return MTL::VertexFormatInt;
                    case BufferDataType::UInt: return MTL::VertexFormatUInt;
                    case BufferDataType::HalfFloat: return MTL::VertexFormatHalf;
                    case BufferDataType::Float: return MTL::VertexFormatFloat;
                    default:
                        throw AT2Exception("Unsupported vertex format");
                };
            case 2:
                switch (type)
                {
                    case BufferDataType::Byte: return normalized ? MTL::VertexFormatChar2Normalized : MTL::VertexFormatChar2;
                    case BufferDataType::UByte: return normalized ? MTL::VertexFormatUChar2Normalized : MTL::VertexFormatUChar2;
                    case BufferDataType::Short: return normalized ? MTL::VertexFormatShort2Normalized : MTL::VertexFormatShort2;
                    case BufferDataType::UShort: return normalized ? MTL::VertexFormatUShort2Normalized : MTL::VertexFormatUShort2;
                    case BufferDataType::Int: return MTL::VertexFormatInt2;
                    case BufferDataType::UInt: return MTL::VertexFormatUInt2;
                    case BufferDataType::HalfFloat: return MTL::VertexFormatHalf2;
                    case BufferDataType::Float: return MTL::VertexFormatFloat2;
                    default:
                        throw AT2Exception("Unsupported vertex format");
                };
            case 3:
                switch (type)
                {
                    case BufferDataType::Byte: return normalized ? MTL::VertexFormatChar3Normalized : MTL::VertexFormatChar3;
                    case BufferDataType::UByte: return normalized ? MTL::VertexFormatUChar3Normalized : MTL::VertexFormatUChar3;
                    case BufferDataType::Short: return normalized ? MTL::VertexFormatShort3Normalized : MTL::VertexFormatShort3;
                    case BufferDataType::UShort: return normalized ? MTL::VertexFormatUShort3Normalized : MTL::VertexFormatUShort3;
                    case BufferDataType::Int: return MTL::VertexFormatInt3;
                    case BufferDataType::UInt: return MTL::VertexFormatUInt3;
                    case BufferDataType::HalfFloat: return MTL::VertexFormatHalf3;
                    case BufferDataType::Float: return MTL::VertexFormatFloat3;
                    default:
                        throw AT2Exception("Unsupported vertex format");
                };
            case 4:
                switch (type)
                {
                    case BufferDataType::Byte: return normalized ? MTL::VertexFormatChar4Normalized : MTL::VertexFormatChar4;
                    case BufferDataType::UByte: return normalized ? MTL::VertexFormatUChar4Normalized : MTL::VertexFormatUChar4;
                    case BufferDataType::Short: return normalized ? MTL::VertexFormatShort4Normalized : MTL::VertexFormatShort4;
                    case BufferDataType::UShort: return normalized ? MTL::VertexFormatUShort4Normalized : MTL::VertexFormatUShort4;
                    case BufferDataType::Int: return MTL::VertexFormatInt4;
                    case BufferDataType::UInt: return MTL::VertexFormatUInt4;
                    case BufferDataType::HalfFloat: return MTL::VertexFormatHalf4;
                    case BufferDataType::Float: return MTL::VertexFormatFloat4;
                    default:
                        throw AT2Exception("Unsupported vertex format");
                };
            default:
                throw AT2Exception("Unsupported vertex format: Count must be in range 1..4");
        }
    }

constexpr MTL::IndexType TranslateIndexBufferType(BufferDataType type)
{
    switch (type)
    {
        case BufferDataType::UShort: return MTL::IndexTypeUInt16;
        case BufferDataType::UInt: return MTL::IndexTypeUInt32;
        default:
            throw AT2Exception("Metal supports only UInt16 and UInt32 index buffer types");
    }
}

constexpr MTL::ResourceOptions TranslateBufferType(VertexBufferType bufferUsage)
{
    //using namespace Flags;
    assert((bufferUsage & VertexBufferType{VertexBufferFlags::Dynamic, VertexBufferFlags::Stream}).Count() <= 1);
    
    if ((bufferUsage & VertexBufferType{VertexBufferFlags::Dynamic, VertexBufferFlags::Stream}).Any())
    {
#if defined(TARGET_OS_IPHONE)
        return MTL::ResourceCPUCacheModeWriteCombined|MTL::ResourceStorageModeManaged;
#else
        return MTL::ResourceCPUCacheModeWriteCombined|MTL::ResourceStorageModeShared;
#endif
    }
    else //Immutable
    {
#if defined(TARGET_OS_IPHONE)
        return MTL::ResourceStorageModeManaged;
#else
        return MTL::ResourceStorageModeShared;
#endif
    }
}


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

    constexpr MTL::CompareFunction TranslateCompareFunction(CompareFunction function)
    {
        switch (function)
        {
            case CompareFunction::Always: return MTL::CompareFunctionAlways;
            case CompareFunction::Equal: return MTL::CompareFunctionEqual;
            case CompareFunction::NotEqual: return MTL::CompareFunctionNotEqual;
            case CompareFunction::Greater: return MTL::CompareFunctionGreater;
            case CompareFunction::GreaterEqual: return MTL::CompareFunctionGreaterEqual;
            case CompareFunction::Less: return MTL::CompareFunctionLess;
            case CompareFunction::LessEqual: return MTL::CompareFunctionLessEqual;
            case CompareFunction::Never: return MTL::CompareFunctionNever;
            default:
                throw AT2Exception("Wrong CompareFunction value");
        }
    }

    constexpr MTL::BlendFactor TranslateBlendFactor(BlendFactor factor)
    {
        switch (factor)
        {
            case BlendFactor::Zero: return MTL::BlendFactorZero;
            case BlendFactor::One: return MTL::BlendFactorOne;
            case BlendFactor::SourceColor: return MTL::BlendFactorSourceColor;
            case BlendFactor::OneMinusSourceColor: return MTL::BlendFactorOneMinusSourceColor;
            case BlendFactor::DestinationColor: return MTL::BlendFactorDestinationColor;
            case BlendFactor::OneMinusDestinationColor: return MTL::BlendFactorOneMinusDestinationColor;
            case BlendFactor::SourceAlpha: return MTL::BlendFactorSourceAlpha;
            case BlendFactor::OneMinusSourceAlpha: return MTL::BlendFactorOneMinusSourceAlpha;
            case BlendFactor::DestinationAlpha: return MTL::BlendFactorDestinationAlpha;
            case BlendFactor::OneMinusDestinationAlpha: return MTL::BlendFactorOneMinusDestinationAlpha;
            case BlendFactor::ConstantColor: return MTL::BlendFactorBlendColor;
            case BlendFactor::OneMinusConstantColor: return MTL::BlendFactorOneMinusBlendColor;
            case BlendFactor::ConstantAlpha: return MTL::BlendFactorBlendAlpha;
            case BlendFactor::OneMinusConstantAlpha: return MTL::BlendFactorOneMinusBlendAlpha;
            default:
                throw AT2Exception("Wrong BlendFactor value");
        }
    }

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
            case PolygonRasterizationMode::Lines: return MTL::TriangleFillModeLines;
            case PolygonRasterizationMode::Fill: return MTL::TriangleFillModeFill;
            case PolygonRasterizationMode::Point:
                break;
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

    constexpr MTL::VertexStepFunction TranslateVertexStepFunc(VertexStepFunc stepFunc)
    {
        switch (stepFunc)
        {
            case VertexStepFunc::PerVertex:     return MTL::VertexStepFunctionPerVertex;
            case VertexStepFunc::PerInstance:   return MTL::VertexStepFunctionPerInstance;
        }

        throw AT2Exception("Unsupported VertexStepFunc");
    }
/*
    constexpr GLboolean TranslateBool(bool value)
    {
        return value ? GL_TRUE : GL_FALSE;
    }
 */

} // namespace AT2::Mappings
