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

    //TODO: test all cases
    std::visit(
        Utils::overloaded {
            [&](const Texture1D& texture) {
                m_size = glm::ivec3 {texture.getSize(), 1, 1};
                glTextureStorage1D(m_id, static_cast<GLint>(texture.getLevels()), m_internalFormat, m_size.x);

                SetWrapMode(TextureWrapParams::Uniform(TextureWrapMode::Repeat));
                SetSamplingMode(TextureSamplingParams::Uniform(TextureSamplingMode::Linear, texture.getLevels() > 1));
            },
            [&](const Texture1DArray& texture) {
                m_size = glm::ivec3 {texture.getSize(), 1};
                glTextureStorage2D(m_id, static_cast<GLint>(texture.getLevels()), m_internalFormat, m_size.x, m_size.y);

                SetWrapMode(TextureWrapParams::Uniform(TextureWrapMode::Repeat));
                SetSamplingMode(TextureSamplingParams::Uniform(TextureSamplingMode::Linear, texture.getLevels() > 1));
            },
            [&](const Texture2D& texture) {
                m_size = glm::ivec3 {texture.getSize(), 1};
                glTextureStorage2D(m_id, static_cast<GLint>(texture.getLevels()), m_internalFormat, m_size.x, m_size.y);

                SetWrapMode(TextureWrapParams::Uniform(TextureWrapMode::Repeat));
                SetSamplingMode(TextureSamplingParams::Uniform(TextureSamplingMode::Linear, texture.getLevels() > 1));
            },
            [&](const Texture2DMultisample& texture) {
                m_size = glm::ivec3 {texture.getSize(), 1};
                glTextureStorage2DMultisample(m_id, static_cast<GLint>(texture.getLevels()), m_internalFormat, m_size.x,
                                              m_size.y, texture.getFixedSampleLocations());

                SetWrapMode(TextureWrapParams::Uniform(TextureWrapMode::Repeat));
                SetSamplingMode(TextureSamplingParams::Uniform(TextureSamplingMode::Linear, texture.getLevels() > 1));
            },
            [=](const Texture2DRectangle& texture) {
                m_size = glm::ivec3 {texture.getSize(), 1};
                glTextureStorage2D(m_id, static_cast<GLint>(texture.getLevels()), m_internalFormat, m_size.x, m_size.y);

                SetWrapMode(TextureWrapParams::Uniform(TextureWrapMode::Repeat));
                SetSamplingMode(TextureSamplingParams::Uniform(TextureSamplingMode::Linear, texture.getLevels() > 1));
            },
            [&](const Texture2DArray& texture) {
                m_size = glm::ivec3 {texture.getSize()};
                glTextureStorage3D(m_id, static_cast<GLint>(texture.getLevels()), m_internalFormat, m_size.x, m_size.y, m_size.z);

                SetWrapMode(TextureWrapParams::Uniform(TextureWrapMode::Repeat));
                SetSamplingMode(TextureSamplingParams::Uniform(TextureSamplingMode::Linear, texture.getLevels() > 1));
            },
            [&](const Texture2DMultisampleArray& texture) {
                m_size = glm::ivec3 {texture.getSize()};
                glTextureStorage3DMultisample(m_id, static_cast<GLint>(texture.getSamples()), m_internalFormat,
                                              m_size.x, m_size.y, m_size.z, texture.getFixedSampleLocations());

                SetWrapMode(TextureWrapParams::Uniform(TextureWrapMode::Repeat));
                SetSamplingMode(TextureSamplingParams::Uniform(TextureSamplingMode::Linear, texture.getLevels() > 1));
            },
            [&](const TextureCube& texture) {
                m_size = glm::ivec3 {texture.getSize(), 1};
                glTextureStorage2D(m_id, static_cast<GLint>(texture.getLevels()), m_internalFormat, m_size.x, m_size.y);

                SetWrapMode( TextureWrapParams::Uniform(TextureWrapMode::ClampToEdge));
                SetSamplingMode(TextureSamplingParams::Uniform(TextureSamplingMode::Linear, texture.getLevels() > 1));
            },
            [&](const TextureCubeArray& texture) {
                m_size = glm::ivec3 {texture.getSize()};
                glTextureStorage3D(m_id, static_cast<GLint>(texture.getLevels()), m_internalFormat, m_size.x, m_size.y,
                                   m_size.z);

                SetWrapMode(TextureWrapParams::Uniform(TextureWrapMode::ClampToEdge));
                SetSamplingMode(TextureSamplingParams::Uniform(TextureSamplingMode::Linear, texture.getLevels() > 1));
            },
            [&](const Texture3D& texture) {
                m_size = glm::ivec3 {texture.getSize()};
                glTextureStorage3D(m_id, static_cast<GLint>(texture.getLevels()), m_internalFormat, m_size.x, m_size.y,
                                   m_size.z);

                SetWrapMode(TextureWrapParams::Uniform(TextureWrapMode::Repeat));
                SetSamplingMode(TextureSamplingParams::Uniform(TextureSamplingMode::Linear, texture.getLevels() > 1));
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
    glBindTextureUnit(unit, m_id);

    m_currentTextureModule = unit;
}

void GlTexture::BindAsImage(unsigned unit, glm::u32 level, glm::u32 layer, bool isLayered, BufferUsage usage) const
{
    glBindImageTexture(unit, GetId(), static_cast<GLint>(level), isLayered, static_cast<GLint>(layer), Mappings::TranslateBufferUsage(usage),
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

void GlTexture::SetWrapMode(TextureWrapParams wrapParams)
{
    m_wrapParams = wrapParams;

    glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, Mappings::TranslateWrapMode(wrapParams.WrapS));
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, Mappings::TranslateWrapMode(wrapParams.WrapT));
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_R, Mappings::TranslateWrapMode(wrapParams.WrapR));
}

void GlTexture::SetSamplingMode(TextureSamplingParams samplingParams)
{
    m_sampling_params = samplingParams;

    glTextureParameteri(m_id, GL_TEXTURE_MAX_LEVEL, Mappings::TranslateTextureSamplingModes(samplingParams.Magnification));
    glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, std::apply(Mappings::TranslateTextureSamplingModes, samplingParams.Minification));
}

void GlTexture::SubImage1D(glm::u32 _offset, glm::u32 _size, glm::u32 _level, ExternalTextureFormat dataFormat, const void* data)
{
    const auto externalFormat = Mappings::TranslateExternalFormat(dataFormat.ChannelsLayout);
    const auto externalType = Mappings::TranslateExternalType(dataFormat.DataType);

    const auto offset = static_cast<GLint> (_offset);
    const auto size = static_cast<GLint> (_size);
    const auto level = static_cast<GLint>(_level);

    if (size + offset > m_size.x)
        throw AT2Exception(AT2Exception::ErrorCase::Texture, "Some SubImage texels out of texture bounds");

    if (const auto* tex1D = std::get_if<Texture1D>(&GetType()))
    {
        if (size >= static_cast<int>(tex1D->getSize().x))
            throw AT2Exception(AT2Exception::ErrorCase::Texture, "SubImage size more than texture actual size");

        glTextureSubImage1D(m_id, level, m_internalFormat, size, externalFormat, externalType, data);
    }
    else
        throw AT2Exception(AT2Exception::ErrorCase::NotImplemented,
                           "SubImage1D operation could be performed only at Texture1D target");
}

void GlTexture::SubImage2D(glm::uvec2 _offset, glm::uvec2 _size, glm::u32 _level, ExternalTextureFormat dataFormat,
                           const void* data)
{
    const auto externalFormat = Mappings::TranslateExternalFormat(dataFormat.ChannelsLayout);
    const auto externalType = Mappings::TranslateExternalType(dataFormat.DataType);

    const auto offset = glm::ivec2 {_offset};
    const auto size = glm::ivec2 {_size};
    const auto level = static_cast<GLint>(_level);

    if (const auto maxCoord = size + offset; maxCoord.x > m_size.x || maxCoord.y > m_size.y)
        throw AT2Exception(AT2Exception::ErrorCase::Texture, "Some SubImage texels out of texture bounds");

    using namespace std;

    visit(
        [=, id=m_id]<typename T>(T) {
            if constexpr (is_same_v<T, Texture1DArray> || is_same_v<T, Texture2D>)
                glTextureSubImage2D(id, level, offset.x, offset.y, size.x, size.y, externalFormat,
                                       externalType, data);
            else
                throw AT2Exception(AT2Exception::ErrorCase::NotImplemented,
                                   "SubImage2D supports only Texture1DArray, Texture2D, TextureCube");
        },
        GetType());
}

void GlTexture::SubImage3D(glm::uvec3 _offset, glm::uvec3 _size, glm::u32 _level, ExternalTextureFormat dataFormat,
                           const void* data)
{
    const auto externalFormat = Mappings::TranslateExternalFormat(dataFormat.ChannelsLayout);
    const auto externalType = Mappings::TranslateExternalType(dataFormat.DataType);

    const auto offset = glm::ivec3{_offset};
    const auto size = glm::ivec3{_size};
    const auto level = static_cast<GLint>(_level);

    if (const auto maxCoord = size + offset;
        maxCoord.x > m_size.x || maxCoord.y > m_size.y || maxCoord.z > m_size.z)
        throw AT2Exception(AT2Exception::ErrorCase::Texture, "Some SubImage texels out of texture bounds");

    using namespace std;
    visit(
        [=, id=m_id]<typename T>(T) {
            if constexpr (is_same_v<T, Texture2DArray> || is_same_v<T, Texture3D> || is_same_v<T, TextureCubeArray> ||
                          is_same_v<T, TextureCube>)
            {
                if constexpr (is_same_v<T, TextureCube>)
                {
                    if (size.z > 5)
                        throw AT2Exception("GlTexture:SubImage3D cube map face must be in range [0-5]");
                }

                glTextureSubImage3D(id, level, offset.x, offset.y, offset.z, size.x, size.y, size.z, externalFormat,
                                    externalType, data);
            }
            else
                throw AT2Exception(AT2Exception::ErrorCase::NotImplemented,
                                   "SubImage3D supports only Texture2DArray, Texture3D");
        },
        GetType());
}

void GlTexture::CopyFromFramebuffer(GLint level, glm::ivec2 pos, glm::ivec2 size, glm::ivec3 textureOffset)
{
    if (const auto maxCoord = textureOffset + glm::ivec3(size, 1);
        maxCoord.x > m_size.x || maxCoord.y > m_size.y || maxCoord.z > m_size.z)
        throw AT2Exception(AT2Exception::ErrorCase::Texture, "Some CopyFromFramebuffer texels out of texture bounds");

    using namespace std;

    visit(
        [=, id = m_id]<typename T>(T) {
            if constexpr (is_same_v<T, Texture1DArray> || is_same_v<T, Texture2D> || is_same_v<T, Texture2DRectangle>)
                glCopyTextureSubImage2D(id, level, textureOffset.x, textureOffset.y, pos.x, pos.y, size.x, size.y);
            else if constexpr (is_same_v<T, Texture3D> || is_same_v<T, Texture2DArray> ||
                               is_same_v<T, TextureCubeArray> || is_same_v<T, TextureCube>)
                glCopyTextureSubImage3D(id, level, textureOffset.x, textureOffset.y, textureOffset.z, pos.x, pos.y, size.x, size.y);
            else
                throw AT2Exception(AT2Exception::ErrorCase::NotImplemented, "Probably not implemented");
        },
        GetType());
}
