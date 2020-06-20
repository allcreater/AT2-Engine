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
    case TextureWrapMode::ClampToEdge:          return GL_CLAMP_TO_EDGE;
    case TextureWrapMode::ClampToBorder:        return GL_CLAMP_TO_BORDER;
    case TextureWrapMode::MirroredRepeat:       return GL_MIRRORED_REPEAT;
    case TextureWrapMode::Repeat:               return GL_REPEAT;
    case TextureWrapMode::MirrorClampToEdge:    return GL_MIRROR_CLAMP_TO_EDGE;
    default:
        assert(false);
    }

    return 0;
}

inline GLenum TranslateExternalFormat(TextureLayout layout)
{
    switch (layout)
    {
    case TextureLayout::Red:            return GL_RED;
    case TextureLayout::RG:             return GL_RG;
    case TextureLayout::RGB:            return GL_RGB;
    case TextureLayout::BGR:            return GL_BGR;
    case TextureLayout::BGRA:           return GL_BGRA;
    case TextureLayout::RGBA:           return GL_RGBA;
    case TextureLayout::DepthComponent: return GL_DEPTH_COMPONENT;
    case TextureLayout::StencilIndex:   return GL_STENCIL_INDEX;
    default:
        assert(false);
    }

    return 0;
}

inline GLenum TranslateExternalType(BufferDataType type)
{
    switch (type)
    {
    case BufferDataType::Byte:      return GL_BYTE;
    case BufferDataType::UByte:     return GL_UNSIGNED_BYTE;
    case BufferDataType::Short:     return GL_SHORT;
    case BufferDataType::UShort:    return GL_UNSIGNED_SHORT;
    case BufferDataType::Int:       return GL_INT;
    case BufferDataType::UInt:      return GL_UNSIGNED_INT;
    case BufferDataType::HalfFloat: return GL_HALF_FLOAT;
    case BufferDataType::Float:     return GL_FLOAT;
    case BufferDataType::Double:    return GL_DOUBLE;
    case BufferDataType::Fixed:     return GL_FIXED;
    default:
        assert(false);
    }

    return 0;
}
inline GLenum TranslateBufferType(VertexBufferType bufferType)
{
    switch (bufferType)
    {
    case VertexBufferType::ArrayBuffer: return GL_ARRAY_BUFFER;
    case VertexBufferType::IndexBuffer: return GL_ELEMENT_ARRAY_BUFFER;
    case VertexBufferType::UniformBuffer: return GL_UNIFORM_BUFFER;
    default:
        assert(false); //not supported yet :(
    }

    return 0;
}

inline GLenum TranslateBufferUsage (BufferUsage bufferUsage)
{
    switch (bufferUsage)
    {
    case BufferUsage::ReadOnly: return GL_READ_ONLY;
    case BufferUsage::ReadWrite: return GL_READ_WRITE;
    case BufferUsage::WriteOnly: return GL_WRITE_ONLY;
    default:
        assert(false);
    }

    return 0;
}

inline GLenum TranslateTextureTarget(const Texture& texture)
{
    return std::visit(Utils::overloaded {
        [](const Texture1D& )                   { return GL_TEXTURE_1D;                     },
        [](const Texture1DArray& )              { return GL_TEXTURE_1D_ARRAY;               },
        [](const Texture2DArray& )              { return GL_TEXTURE_2D_ARRAY;               },
        [](const Texture2D& )                   { return GL_TEXTURE_2D;                     },
        [](const Texture2DMultisample& )        { return GL_TEXTURE_2D_MULTISAMPLE;         },
        [](const Texture2DRectangle& )          { return GL_TEXTURE_RECTANGLE;              },
        [](const Texture2DMultisampleArray& )   { return GL_TEXTURE_2D_MULTISAMPLE_ARRAY;   },
        [](const TextureCube& )                 { return GL_TEXTURE_CUBE_MAP;               },
        [](const TextureCubeArray& )            { return GL_TEXTURE_CUBE_MAP_ARRAY;         },
        [](const Texture3D& )                   { return GL_TEXTURE_3D;                     }
        }, texture);
}

inline GLenum TranslatePrimitiveType (const Primitives::Primitive& primitive)
{
    using namespace Primitives;

    return std::visit(Utils::overloaded {
        [](const Points&)                       { return GL_POINTS;                     },
        [](const LineStrip&)                    { return GL_LINE_STRIP;                 },
        [](const LineLoop&)                     { return GL_LINE_LOOP;                  },
        [](const Lines&)                        { return GL_LINES;                      },
        [](const LineStripAdjacency&)           { return GL_LINE_STRIP_ADJACENCY;       },
        [](const LinesAdjacency&)               { return GL_LINES_ADJACENCY;            },
        [](const TriangleStrip&)                { return GL_TRIANGLE_STRIP;             },
        [](const TriangleFan&)                  { return GL_TRIANGLE_FAN;               },
        [](const Triangles&)                    { return GL_TRIANGLES;                  },
        [](const TriangleStripAdjacency&)       { return GL_TRIANGLE_STRIP_ADJACENCY;   },
        [](const TrianglesAdjacency&)           { return GL_TRIANGLES_ADJACENCY;        },
        [](const Patches&)                      { return GL_PATCHES;                    },
    }, primitive);
}

}