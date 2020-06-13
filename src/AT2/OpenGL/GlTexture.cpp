#include "GlTexture.h"

#include <algorithm>
#include <optional>

using namespace AT2;

static GLint translateWrapMode(TextureWrapMode wrapMode)
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

static GLenum translateExternalFormat(TextureLayout layout)
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

static GLenum translateExternalType(BufferDataType type)
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

GLenum GlTexture::GetTarget() const
{
    return std::visit( Utils::overloaded {
        [](const Texture1D& texture)                   { return GL_TEXTURE_1D;                     },
        [](const Texture1DArray& texture)              { return GL_TEXTURE_1D_ARRAY;               },
        [](const Texture2DArray& texture)              { return GL_TEXTURE_2D_ARRAY;               },
        [](const Texture2D& texture)                   { return GL_TEXTURE_2D;                     },
        [](const Texture2DMultisample& texture)        { return GL_TEXTURE_2D_MULTISAMPLE;         },
        [](const Texture2DRectangle& texture)          { return GL_TEXTURE_RECTANGLE;              },
        [](const Texture2DMultisampleArray& texture)   { return GL_TEXTURE_2D_MULTISAMPLE_ARRAY;   },
        [](const TextureCube& texture)                 { return GL_TEXTURE_CUBE_MAP;               },
        [](const TextureCubeArray& texture)            { return GL_TEXTURE_CUBE_MAP_ARRAY;         },
        [](const Texture3D& texture)                   { return GL_TEXTURE_3D;                     }
    }, m_flavor);
}


GlTexture::GlTexture(Texture flavor, GLint internalFormat) : m_flavor(flavor), m_internalFormat(internalFormat)
{
    glGenTextures(1, &m_id);

    const auto target = GetTarget();

    glTextureParameteriEXT(m_id, target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteriEXT(m_id, target, GL_TEXTURE_BASE_LEVEL, 0);

    //TODO: test all cases
    //todo: remove EXT?
    std::visit(Utils::overloaded{
        [=](const Texture1D& texture)
        {
            m_size = { texture.getSize(), 1, 1 };
            glTextureStorage1DEXT(m_id, GL_TEXTURE_1D, texture.getLevels(), m_internalFormat , texture.getSize().x);

            glTextureParameteriEXT(m_id, target, GL_TEXTURE_MIN_FILTER, (texture.getLevels() > 1) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
            glTextureParameteriEXT(m_id, target, GL_TEXTURE_MAX_LEVEL, texture.getLevels());
        },
        [=](const Texture1DArray& texture)
        {
            m_size = { texture.getSize(), 1 };
            glTextureStorage2DEXT(m_id, GL_TEXTURE_1D_ARRAY, texture.getLevels(), m_internalFormat, texture.getSize().x, texture.getSize().y);
        },
        [=](const Texture2D& texture)
        {
            m_size = { texture.getSize(), 1 };
            glTextureStorage2DEXT(m_id, GL_TEXTURE_2D, texture.getLevels(), m_internalFormat, texture.getSize().x, texture.getSize().y);

            glTextureParameteriEXT(m_id, target, GL_TEXTURE_MIN_FILTER, (texture.getLevels() > 1) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
            glTextureParameteriEXT(m_id, target, GL_TEXTURE_MAX_LEVEL, texture.getLevels());
        },
        [=](const Texture2DMultisample& texture)
        {
            m_size = { texture.getSize(), 1 };
            glTextureStorage2DMultisample(m_id, texture.getSamples(), m_internalFormat, texture.getSize().x, texture.getSize().y, texture.getFixedSampleLocations());
        },
        [=](const Texture2DRectangle& texture)
        {
            m_size = { texture.getSize(), 1 };
            glTextureStorage2DEXT(m_id, GL_TEXTURE_RECTANGLE, texture.getLevels(), m_internalFormat, texture.getSize().x, texture.getSize().y);
        },
        [=](const Texture2DArray& texture)
        {
            m_size = texture.getSize();
            glTextureStorage3DEXT(m_id, GL_TEXTURE_2D_ARRAY, texture.getLevels(), m_internalFormat, texture.getSize().x, texture.getSize().y, texture.getSize().z);

            glTextureParameteriEXT(m_id, target, GL_TEXTURE_MIN_FILTER, (texture.getLevels() > 1) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
            glTextureParameteriEXT(m_id, target, GL_TEXTURE_MAX_LEVEL, texture.getLevels());
        },
        [=](const Texture2DMultisampleArray& texture)
        {
            m_size = texture.getSize();
            glTextureStorage3DMultisample(m_id, texture.getSamples(), m_internalFormat, texture.getSize().x, texture.getSize().y, texture.getSize().z, texture.getFixedSampleLocations());
        },
        [=](const TextureCube& texture)
        {
            m_size = { texture.getSize(), 1};
            glTextureStorage2DEXT(m_id, GL_TEXTURE_CUBE_MAP, texture.getLevels(), m_internalFormat, texture.getSize().x, texture.getSize().y);

            glTextureParameteriEXT(m_id, target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteriEXT(m_id, target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTextureParameteriEXT(m_id, target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

            glTextureParameteriEXT(m_id, target, GL_TEXTURE_MIN_FILTER, (texture.getLevels() > 1) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
            glTextureParameteriEXT(m_id, target, GL_TEXTURE_MAX_LEVEL, texture.getLevels());
        },
        [=](const TextureCubeArray& texture)
        {
            m_size = texture.getSize();
            glTextureStorage3DEXT(m_id, GL_TEXTURE_CUBE_MAP_ARRAY, texture.getLevels(), m_internalFormat, texture.getSize().x, texture.getSize().y, texture.getSize().z);
        },
        [=](const Texture3D& texture)
        {
            m_size = texture.getSize();
            glTextureStorage3DEXT(m_id, GL_TEXTURE_3D, texture.getLevels(), m_internalFormat, texture.getSize().x, texture.getSize().y, texture.getSize().z);
            glTextureParameteriEXT(m_id, target, GL_TEXTURE_MIN_FILTER, (texture.getLevels() > 1) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
            glTextureParameteriEXT(m_id, target, GL_TEXTURE_MAX_LEVEL, texture.getLevels());
        }
    }, flavor);

}

GlTexture::~GlTexture()
{
    glDeleteTextures(1, &m_id);
}

void GlTexture::Bind(unsigned int module)
{
    glActiveTexture(GL_TEXTURE0 + module);
    glBindTexture(GetTarget(), m_id);

    m_currentTextureModule = module;
}

void GlTexture::Unbind()
{
    m_currentTextureModule = -1;
}

void GlTexture::BuildMipmaps()
{
    glGenerateTextureMipmapEXT(m_id, GetTarget());
}

void GlTexture::SetWrapMode(TextureWrapMode wrapMode)
{
    //TODO: use dirty flag and set it on bind
    m_wrapMode = wrapMode;

    const auto target = GetTarget();
    const auto mode = translateWrapMode(wrapMode);

    glTextureParameteriEXT(m_id, target, GL_TEXTURE_WRAP_S, mode);
    glTextureParameteriEXT(m_id, target, GL_TEXTURE_WRAP_T, mode);
    glTextureParameteriEXT(m_id, target, GL_TEXTURE_WRAP_R, mode);
}

void GlTexture::SubImage1D(glm::u32 offset, glm::u32 size, glm::u32 level, ExternalTextureFormat dataFormat, void* data)
{
    const auto externalFormat = translateExternalFormat(dataFormat.ChannelsLayout);
    const auto externalType = translateExternalType(dataFormat.DataType);

    if (size + offset > GetSize().x)
        throw AT2Exception(AT2Exception::ErrorCase::Texture, "Some SubImage texels out of texture bounds");

    if (const auto* tex1D = std::get_if<Texture1D>(&GetType()))
    {
        if (size >= tex1D->getSize().x)
            throw AT2Exception(AT2Exception::ErrorCase::Texture, "SubImage size more than texture actual size");

        glTextureSubImage1DEXT(m_id, GL_TEXTURE_1D, level, m_internalFormat, size, externalFormat, externalType, data);
    }
    else
        throw AT2Exception(AT2Exception::ErrorCase::NotImplemented, "SubImage1D operation could be performed only at Texture1D target");
}

void GlTexture::SubImage2D(glm::uvec2 offset, glm::uvec2 size, glm::u32 level, ExternalTextureFormat dataFormat, void* data, int cubeMapFace)
{
    const auto externalFormat = translateExternalFormat(dataFormat.ChannelsLayout);
    const auto externalType = translateExternalType(dataFormat.DataType);

    if (cubeMapFace < 0 || cubeMapFace > 5)
        throw AT2Exception("GlTexture:SubImage2D cube map face must be in range [0-5]");

    if (const auto maxCoord = size + offset; maxCoord.x > GetSize().x || maxCoord.y > GetSize().y)
        throw AT2Exception(AT2Exception::ErrorCase::Texture, "Some SubImage texels out of texture bounds");

    using namespace std;

    visit([=](const auto& type) {
        using T = std::decay_t<decltype(type)>;
        if constexpr (is_same_v<T, Texture1DArray> || is_same_v<T, Texture2D>)
            glTextureSubImage2DEXT(m_id, GetTarget(), level, offset.x, offset.y, size.x, size.y, externalFormat, externalType, data);
        else if constexpr (is_same_v<T, TextureCube>)
            glTextureSubImage2DEXT(m_id, GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubeMapFace, level, offset.x, offset.y, size.x, size.y, externalFormat, externalType, data);
        else
            throw AT2Exception(AT2Exception::ErrorCase::NotImplemented, "SubImage2D supports only Texture1DArray, Texture2D, TextureCube");
    }, GetType());
}

void GlTexture::SubImage3D(glm::uvec3 offset, glm::uvec3 size, glm::u32 level, ExternalTextureFormat dataFormat, void* data)
{
    const auto externalFormat = translateExternalFormat(dataFormat.ChannelsLayout);
    const auto externalType = translateExternalType(dataFormat.DataType);

    if (const auto maxCoord = size + offset; maxCoord.x > GetSize().x || maxCoord.y > GetSize().y || maxCoord.z > GetSize().z)
        throw AT2Exception(AT2Exception::ErrorCase::Texture, "Some SubImage texels out of texture bounds");

    using namespace std;
    visit([=](const auto& type) {
        using T = std::decay_t<decltype(type)>;
        if constexpr (is_same_v<T, Texture2DArray> || is_same_v<T, Texture3D>)
            glTextureSubImage3DEXT(m_id, GetTarget(), level, offset.x, offset.y, offset.z, size.x, size.y, size.z, externalFormat, externalType, data);
        else
            throw AT2Exception(AT2Exception::ErrorCase::NotImplemented, "SubImage3D supports only Texture2DArray, Texture3D");
    }, GetType());
}

void GlTexture::CopyFromFramebuffer(GLuint level, glm::ivec2 pos, glm::uvec2 size, int cubeMapFace)
{
    if (const auto maxCoord = size; maxCoord.x > GetSize().x || maxCoord.y > GetSize().y)
        throw AT2Exception(AT2Exception::ErrorCase::Texture, "Some SubImage texels out of texture bounds");

    if (cubeMapFace < 0 || cubeMapFace > 5)
        throw AT2Exception("GlTexture:CopyFromFramebuffer cube map face must be in range [0-5]");

    using namespace std;

    visit([=](const auto & type){
        using T = std::decay_t<decltype(type)>;
        if constexpr (is_same_v<T, Texture1DArray> || is_same_v<T, Texture2D> || is_same_v<T, Texture2DRectangle>)
            glCopyTextureSubImage2DEXT(m_id, GetTarget(), level, 0, 0, pos.x, pos.y, size.x, size.y);
        else if constexpr (is_same_v<T, TextureCube>)
            glCopyTextureSubImage2DEXT(m_id, GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubeMapFace, level, 0, 0, pos.x, pos.y, size.x, size.y);
        else
            throw AT2Exception(AT2Exception::ErrorCase::NotImplemented, "Probably not implemented");
    }, GetType());
}
