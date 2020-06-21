#include "GlTexture.h"
#include "Mappings.h"

using namespace AT2;


GLenum GlTexture::GetTarget() const
{
    return Mappings::TranslateTextureTarget(m_flavor);
}

void GlTexture::ReadChannelSizes()
{
    const auto target = GetTarget();
    glGetInternalformativ(target, m_internalFormat, GL_INTERNALFORMAT_RED_SIZE, 1, &m_channelSizes.red);
    glGetInternalformativ(target, m_internalFormat, GL_INTERNALFORMAT_GREEN_SIZE, 1, &m_channelSizes.green);
    glGetInternalformativ(target, m_internalFormat, GL_INTERNALFORMAT_BLUE_SIZE, 1, &m_channelSizes.blue);
    glGetInternalformativ(target, m_internalFormat, GL_INTERNALFORMAT_ALPHA_SIZE, 1, &m_channelSizes.alpha);
    glGetInternalformativ(target, m_internalFormat, GL_INTERNALFORMAT_DEPTH_SIZE, 1, &m_channelSizes.depth);
    glGetInternalformativ(target, m_internalFormat, GL_INTERNALFORMAT_STENCIL_SIZE, 1, &m_channelSizes.stencil);
    glGetInternalformativ(target, m_internalFormat, GL_INTERNALFORMAT_SHARED_SIZE, 1, &m_channelSizes.shared);

    m_dataSize = m_size.x * m_size.y * m_size.z * m_channelSizes.InBytes();
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

    ReadChannelSizes();
}

GlTexture::~GlTexture()
{
    glDeleteTextures(1, &m_id);
}

void GlTexture::Bind(unsigned int module) const
{
    assert(module < 10000); //

    glActiveTexture(GL_TEXTURE0 + module);
    glBindTexture(GetTarget(), m_id);

    m_currentTextureModule = module;
}

void GlTexture::BindAsImage(unsigned module, glm::u32 level, glm::u32 layer, bool isLayered, BufferUsage usage) const
{
    glBindImageTexture(module, GetId(), level, isLayered, layer, Mappings::TranslateBufferUsage(usage), m_internalFormat);
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
    const auto mode = Mappings::TranslateWrapMode(wrapMode);

    glTextureParameteriEXT(m_id, target, GL_TEXTURE_WRAP_S, mode);
    glTextureParameteriEXT(m_id, target, GL_TEXTURE_WRAP_T, mode);
    glTextureParameteriEXT(m_id, target, GL_TEXTURE_WRAP_R, mode);
}

void GlTexture::SubImage1D(glm::u32 offset, glm::u32 size, glm::u32 level, ExternalTextureFormat dataFormat, void* data)
{
    const auto externalFormat = Mappings::TranslateExternalFormat(dataFormat.ChannelsLayout);
    const auto externalType = Mappings::TranslateExternalType(dataFormat.DataType);

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
    const auto externalFormat = Mappings::TranslateExternalFormat(dataFormat.ChannelsLayout);
    const auto externalType = Mappings::TranslateExternalType(dataFormat.DataType);

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
    const auto externalFormat = Mappings::TranslateExternalFormat(dataFormat.ChannelsLayout);
    const auto externalType = Mappings::TranslateExternalType(dataFormat.DataType);

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
