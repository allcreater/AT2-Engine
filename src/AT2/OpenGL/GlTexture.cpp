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

    m_dataSize = static_cast<size_t>(m_size.x) * m_size.y * m_size.z * m_channelSizes.InBytes();
}


GlTexture::GlTexture(Texture flavor, GLint internalFormat) : m_flavor(flavor), m_internalFormat(internalFormat)
{
    glCreateTextures(GetTarget(), 1, &m_id);

    glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(m_id, GL_TEXTURE_BASE_LEVEL, 0);

    //TODO: test all cases
    std::visit(Utils::overloaded {
                   [=](const Texture1D& texture) {
                       m_size = {texture.getSize(), 1, 1};
                       glTextureStorage1D(m_id, texture.getLevels(), m_internalFormat,
                                             texture.getSize().x);

                       glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER,
                                              (texture.getLevels() > 1) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
                       glTextureParameteri(m_id, GL_TEXTURE_MAX_LEVEL, texture.getLevels());
                   },
                   [=](const Texture1DArray& texture) {
                       m_size = {texture.getSize(), 1};
                       glTextureStorage2D(m_id, texture.getLevels(), m_internalFormat,
                                             texture.getSize().x, texture.getSize().y);
                   },
                   [=](const Texture2D& texture) {
                       m_size = {texture.getSize(), 1};
                       glTextureStorage2D(m_id, texture.getLevels(), m_internalFormat,
                                             texture.getSize().x, texture.getSize().y);

                       glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER,
                                              (texture.getLevels() > 1) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
                       glTextureParameteri(m_id, GL_TEXTURE_MAX_LEVEL, texture.getLevels());
                   },
                   [=](const Texture2DMultisample& texture) {
                       m_size = {texture.getSize(), 1};
                       glTextureStorage2DMultisample(m_id, texture.getSamples(), m_internalFormat, texture.getSize().x,
                                                     texture.getSize().y, texture.getFixedSampleLocations());
                   },
                   [=](const Texture2DRectangle& texture) {
                       m_size = {texture.getSize(), 1};
                       glTextureStorage2D(m_id, texture.getLevels(), m_internalFormat,
                                             texture.getSize().x, texture.getSize().y);
                   },
                   [=](const Texture2DArray& texture) {
                       m_size = texture.getSize();
                       glTextureStorage3D(m_id, texture.getLevels(), m_internalFormat,
                                             texture.getSize().x, texture.getSize().y, texture.getSize().z);

                       glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER,
                                              (texture.getLevels() > 1) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
                       glTextureParameteri(m_id, GL_TEXTURE_MAX_LEVEL, texture.getLevels());
                   },
                   [=](const Texture2DMultisampleArray& texture) {
                       m_size = texture.getSize();
                       glTextureStorage3DMultisample(m_id, texture.getSamples(), m_internalFormat, texture.getSize().x,
                                                     texture.getSize().y, texture.getSize().z,
                                                     texture.getFixedSampleLocations());
                   },
                   [=](const TextureCube& texture) {
                       m_size = {texture.getSize(), 1};
                       glTextureStorage2D(m_id, texture.getLevels(), m_internalFormat,
                                             texture.getSize().x, texture.getSize().y);

                       glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                       glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                       glTextureParameteri(m_id, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

                       glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER,
                                              (texture.getLevels() > 1) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
                       glTextureParameteri(m_id, GL_TEXTURE_MAX_LEVEL, texture.getLevels());
                   },
                   [=](const TextureCubeArray& texture) {
                       m_size = texture.getSize();
                       glTextureStorage3D(m_id, texture.getLevels(), m_internalFormat,
                                             texture.getSize().x, texture.getSize().y, texture.getSize().z);
                   },
                   [=](const Texture3D& texture) {
                       m_size = texture.getSize();
                       glTextureStorage3D(m_id, texture.getLevels(), m_internalFormat,
                                             texture.getSize().x, texture.getSize().y, texture.getSize().z);
                       glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER,
                                              (texture.getLevels() > 1) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
                       glTextureParameteri(m_id, GL_TEXTURE_MAX_LEVEL, texture.getLevels());
                   }},
               flavor);

    ReadChannelSizes();
}

GlTexture::~GlTexture()
{
    glDeleteTextures(1, &m_id);
}

void GlTexture::Bind(unsigned int unit) const
{
    assert(unit < 10000); //

    glBindTextureUnit(unit, m_id);

    m_currentTextureModule = unit;
}

void GlTexture::BindAsImage(unsigned unit, glm::u32 level, glm::u32 layer, bool isLayered, BufferUsage usage) const
{
    glBindImageTexture(unit, GetId(), level, isLayered, layer, Mappings::TranslateBufferUsage(usage),
                       m_internalFormat);
}

void GlTexture::Unbind() const
{
    m_currentTextureModule = -1;
}

void GlTexture::BuildMipmaps()
{
    glGenerateTextureMipmap(m_id);
}

void GlTexture::SetWrapMode(TextureWrapMode wrapMode)
{
    //TODO: use dirty flag and set it on bind
    m_wrapMode = wrapMode;

    const auto mode = Mappings::TranslateWrapMode(wrapMode);

    glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, mode);
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, mode);
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_R, mode);
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

        glTextureSubImage1D(m_id, level, m_internalFormat, size, externalFormat, externalType, data);
    }
    else
        throw AT2Exception(AT2Exception::ErrorCase::NotImplemented,
                           "SubImage1D operation could be performed only at Texture1D target");
}

void GlTexture::SubImage2D(glm::uvec2 offset, glm::uvec2 size, glm::u32 level, ExternalTextureFormat dataFormat,
                           void* data)
{
    const auto externalFormat = Mappings::TranslateExternalFormat(dataFormat.ChannelsLayout);
    const auto externalType = Mappings::TranslateExternalType(dataFormat.DataType);

    if (const auto maxCoord = size + offset; maxCoord.x > GetSize().x || maxCoord.y > GetSize().y)
        throw AT2Exception(AT2Exception::ErrorCase::Texture, "Some SubImage texels out of texture bounds");

    using namespace std;

    visit(
        [=](const auto& type) {
            using T = std::decay_t<decltype(type)>;
            if constexpr (is_same_v<T, Texture1DArray> || is_same_v<T, Texture2D>)
                glTextureSubImage2D(m_id, level, offset.x, offset.y, size.x, size.y, externalFormat,
                                       externalType, data);
            else
                throw AT2Exception(AT2Exception::ErrorCase::NotImplemented,
                                   "SubImage2D supports only Texture1DArray, Texture2D, TextureCube");
        },
        GetType());
}

void GlTexture::SubImage3D(glm::uvec3 offset, glm::uvec3 size, glm::u32 level, ExternalTextureFormat dataFormat,
                           void* data)
{
    const auto externalFormat = Mappings::TranslateExternalFormat(dataFormat.ChannelsLayout);
    const auto externalType = Mappings::TranslateExternalType(dataFormat.DataType);

    if (const auto maxCoord = size + offset;
        maxCoord.x > GetSize().x || maxCoord.y > GetSize().y || maxCoord.z > GetSize().z)
        throw AT2Exception(AT2Exception::ErrorCase::Texture, "Some SubImage texels out of texture bounds");

    using namespace std;
    visit(
        [=](const auto& type) {
            using T = std::decay_t<decltype(type)>;
            if constexpr (is_same_v<T, Texture2DArray> || is_same_v<T, Texture3D> || is_same_v<T, TextureCubeArray> ||
                          is_same_v<T, TextureCube>)
            {
                if constexpr (is_same_v<T, TextureCube>)
                {
                    if (size.z < 0 || size.z > 5)
                        throw AT2Exception("GlTexture:SubImage3D cube map face must be in range [0-5]");
                }

                glTextureSubImage3D(m_id, level, offset.x, offset.y, offset.z, size.x, size.y, size.z, externalFormat,
                                    externalType, data);
            }
            else
                throw AT2Exception(AT2Exception::ErrorCase::NotImplemented,
                                   "SubImage3D supports only Texture2DArray, Texture3D");
        },
        GetType());
}

void GlTexture::CopyFromFramebuffer(GLuint level, glm::ivec2 pos, glm::uvec2 size, glm::uvec3 textureOffset)
{
    if (const auto maxCoord = textureOffset + glm::uvec3(size, 1);
        maxCoord.x > GetSize().x || maxCoord.y > GetSize().y || maxCoord.z > GetSize().z)
        throw AT2Exception(AT2Exception::ErrorCase::Texture, "Some CopyFromFramebuffer texels out of texture bounds");

    using namespace std;

    visit(
        [=](const auto& type) {
            using T = std::decay_t<decltype(type)>;
            if constexpr (is_same_v<T, Texture1DArray> || is_same_v<T, Texture2D> || is_same_v<T, Texture2DRectangle>)
                glCopyTextureSubImage2D(m_id, level, textureOffset.x, textureOffset.y, pos.x, pos.y, size.x, size.y);
            else if constexpr (is_same_v<T, Texture3D> || is_same_v<T, Texture2DArray> ||
                               is_same_v<T, TextureCubeArray> || is_same_v<T, TextureCube>)
                glCopyTextureSubImage3D(m_id, level, textureOffset.x, textureOffset.y, textureOffset.z, pos.x, pos.y, size.x, size.y);
            else
                throw AT2Exception(AT2Exception::ErrorCase::NotImplemented, "Probably not implemented");
        },
        GetType());
}
