#pragma once

#include "../AT2_textures.hpp"
#include "../AT2_types.hpp"
#include "../utils.hpp"

namespace AT2::Mappings
{


inline GLint TranslateWrapMode(TextureWrapMode wrapMode)
{
    switch (wrapMode)
    {
    case TextureWrapMode::ClampToEdge:			return GL_CLAMP_TO_EDGE;
    case TextureWrapMode::ClampToBorder:		return GL_CLAMP_TO_BORDER;
    case TextureWrapMode::MirroredRepeat:		return GL_MIRRORED_REPEAT;
    case TextureWrapMode::Repeat:				return GL_REPEAT;
    case TextureWrapMode::MirrorClampToEdge:	return GL_MIRROR_CLAMP_TO_EDGE;
    default:
        assert(false);
    }

    return 0;
}

inline GLenum TranslateExternalFormat(TextureLayout layout)
{
    switch (layout)
    {
    case TextureLayout::Red: return GL_RED;
    case TextureLayout::RG: return GL_RG;
    case TextureLayout::RGB: return GL_RGB;
    case TextureLayout::BGR: return GL_BGR;
    case TextureLayout::RGBA: return GL_RGBA;
    case TextureLayout::BGRA: return GL_BGRA;
    case TextureLayout::DepthComponent: return GL_DEPTH_COMPONENT;
    case TextureLayout::StencilIndex: return GL_STENCIL_INDEX;
    default:
        assert(false);
    }

    return 0;
}

inline GLenum TranslateExternalType(BufferDataType type)
{
    switch (type)
    {
    case BufferDataType::Byte:		return GL_BYTE;
    case BufferDataType::UByte:		return GL_UNSIGNED_BYTE;
    case BufferDataType::Short:		return GL_SHORT;
    case BufferDataType::UShort:	return GL_UNSIGNED_SHORT;
    case BufferDataType::Int:		return GL_INT;
    case BufferDataType::UInt:		return GL_UNSIGNED_INT;
    case BufferDataType::HalfFloat:	return GL_HALF_FLOAT;
    case BufferDataType::Float:		return GL_FLOAT;
    case BufferDataType::Double:	return GL_DOUBLE;
    case BufferDataType::Fixed:		return GL_FIXED;
    default:
        assert(false);
    }

    return 0;
}

inline GLenum TranslateTextureTarget(const Texture& texture)
{
    return std::visit(Utils::overloaded{
        [](const Texture1D& )                    { return GL_TEXTURE_1D;                     },
        [](const Texture1DArray& )               { return GL_TEXTURE_1D_ARRAY;               },
        [](const Texture2DArray& )               { return GL_TEXTURE_2D_ARRAY;               },
        [](const Texture2D& )                    { return GL_TEXTURE_2D;                     },
        [](const Texture2DMultisample& )         { return GL_TEXTURE_2D_MULTISAMPLE;         },
        [](const Texture2DRectangle& )           { return GL_TEXTURE_RECTANGLE;              },
        [](const Texture2DMultisampleArray& )    { return GL_TEXTURE_2D_MULTISAMPLE_ARRAY;   },
        [](const TextureCube& )                  { return GL_TEXTURE_CUBE_MAP;               },
        [](const TextureCubeArray& )             { return GL_TEXTURE_CUBE_MAP_ARRAY;         },
        [](const Texture3D& )                    { return GL_TEXTURE_3D;                     }
        }, texture);
}

}
