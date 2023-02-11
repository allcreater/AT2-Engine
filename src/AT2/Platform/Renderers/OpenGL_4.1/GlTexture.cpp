#include "GlTexture.h"
#include "GlRenderer.h"
#include "GlStateManager.h"
#include "Mappings.h"

using namespace AT2;
using namespace OpenGL41;

namespace
{
    // TODO: set for different contexts individually?
    Utils::lazy max_anisotropy = [] {
        float result = 1.0f;

        if (GLAD_GL_ARB_texture_filter_anisotropic)
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &result);

        return result;
    };
}

GLenum GlTexture::GetTarget() const
{
    return Mappings::TranslateTextureTarget(m_flavor);
}

void GlTexture::ReadChannelSizes()
{
    // const auto target = GetTarget();
    // glGetInternalformativ(target, m_internalFormat, GL_INTERNALFORMAT_RED_SIZE, 1, &m_channelSizes.red);
    // glGetInternalformativ(target, m_internalFormat, GL_INTERNALFORMAT_GREEN_SIZE, 1, &m_channelSizes.green);
    // glGetInternalformativ(target, m_internalFormat, GL_INTERNALFORMAT_BLUE_SIZE, 1, &m_channelSizes.blue);
    // glGetInternalformativ(target, m_internalFormat, GL_INTERNALFORMAT_ALPHA_SIZE, 1, &m_channelSizes.alpha);
    // glGetInternalformativ(target, m_internalFormat, GL_INTERNALFORMAT_DEPTH_SIZE, 1, &m_channelSizes.depth);
    // glGetInternalformativ(target, m_internalFormat, GL_INTERNALFORMAT_STENCIL_SIZE, 1, &m_channelSizes.stencil);
    // glGetInternalformativ(target, m_internalFormat, GL_INTERNALFORMAT_SHARED_SIZE, 1, &m_channelSizes.shared);

    // m_dataSize = static_cast<size_t>(m_size.x) * m_size.y * m_size.z * m_channelSizes.InBytes();

    m_dataSize = static_cast<size_t>(m_size.x) * m_size.y * m_size.z * getPixelSize(getTextureFormat(m_flavor)); // TODO: check correctness
}

namespace 
{
    void glTexStorage2D_internal(GLenum target, GLsizei levels, GLint internalFormat, GLint width, GLint height, TextureFormat format)
    {
        if (glad_glTexStorage2D)
        {
            glTexStorage2D(target, levels, internalFormat, width, height);
            return;
        }

        const auto [externalFormat, externalType] = Mappings::TranslateTextureExternalFormatAndType(format);
        for (GLint i = 0; i < levels; i++)
        {
            glTexImage2D(target, i, internalFormat, width, height, 0, externalFormat, externalType, nullptr);
            width = std::max(1, (width / 2));
            height = std::max(1, (height / 2));
        }
    }   
    
    void glTexStorage3D_internal(GLenum target, GLsizei levels, GLint internalFormat, GLint width, GLint height, GLint depth, TextureFormat format)
    {
        if (glad_glTexStorage3D)
        {
            glTexStorage3D(target, levels, internalFormat, width, height, depth);
            return;
        }

        const auto [externalFormat, externalType] = Mappings::TranslateTextureExternalFormatAndType(format);
        for (GLint i = 0; i < levels; i++)
        {
            glTexImage3D(target, i, internalFormat, width, height, depth, 0, externalFormat, externalType, nullptr);

            width = std::max(1, (width / 2));
            height = std::max(1, (height / 2));
            depth = std::max(1, (depth / 2));
        }
    }
}

std::shared_ptr<GlTexture> GlTexture::Make(GlRenderer& renderer, Texture flavor)
{
    std::shared_ptr<GlTexture> texture{new GlTexture(renderer, flavor)};
    texture->Init();

    return texture;
}

GlTexture::GlTexture(GlRenderer& renderer, Texture flavor)
 : m_stateManager{Utils::safe_dereference_cast<GlStateManager&>(&renderer.GetStateManager())}
 , m_flavor(flavor)
 , m_internalFormat(Mappings::TranslateTextureFormat(getTextureFormat(flavor)))
{
    glGenTextures(1, &m_id);
    if (m_id == 0)
        throw AT2TextureException("Can't create new texture");
}

void GlTexture::Init()
{
    const auto target = GetTarget();
    const auto format = getTextureFormat(m_flavor);
    m_stateManager.DoBind(shared_from_this());

    //TODO: test all cases
    std::visit(
        Utils::overloaded {
            [&](const Texture1D& texture) {
                throw AT2TextureException("Not supported");
                // m_size = glm::ivec3 {texture.getSize(), 1, 1};
                // glTexStorage1D(target, static_cast<GLint>(texture.getLevels()), m_internalFormat, m_size.x);

                // SetWrapMode(TextureWrapParams::Uniform(TextureWrapMode::Repeat));
                // SetSamplingMode(TextureSamplingParams::Uniform(TextureSamplingMode::Linear, texture.getLevels() > 1));
            },
            [&](const Texture1DArray& texture) {
                m_size = glm::ivec3 {texture.getSize(), 1};
                glTexStorage2D_internal(target, static_cast<GLint>(texture.getLevels()), m_internalFormat, m_size.x, m_size.y, format);

                SetWrapMode(TextureWrapParams::Uniform(TextureWrapMode::Repeat));
                SetSamplingMode(TextureSamplingParams::Uniform(TextureSamplingMode::Linear, texture.getLevels() > 1));
            },
            [&](const Texture2D& texture) {
                m_size = glm::ivec3 {texture.getSize(), 1};
                glTexStorage2D_internal(target, static_cast<GLint>(texture.getLevels()), m_internalFormat, m_size.x, m_size.y, format);

                SetWrapMode(TextureWrapParams::Uniform(TextureWrapMode::Repeat));
                SetSamplingMode(TextureSamplingParams::Uniform(TextureSamplingMode::Linear, texture.getLevels() > 1));
            },
            [&](const Texture2DMultisample& texture) {
                throw AT2TextureException("Not supported");
                // m_size = glm::ivec3 {texture.getSize(), 1};
                //        glTexStorage2DMultisample(target, static_cast<GLint>(texture.getSamples()), m_internalFormat, m_size.x,
                //                               m_size.y, texture.getFixedSampleLocations());

                // SetWrapMode(TextureWrapParams::Uniform(TextureWrapMode::Repeat));
                // SetSamplingMode(TextureSamplingParams::Uniform(TextureSamplingMode::Linear));
            },
            [=](const Texture2DRectangle& texture) {
                m_size = glm::ivec3 {texture.getSize(), 1};
                glTexStorage2D_internal(target, static_cast<GLint>(texture.getLevels()), m_internalFormat, m_size.x, m_size.y, format);

                SetWrapMode(TextureWrapParams::Uniform(TextureWrapMode::Repeat));
                SetSamplingMode(TextureSamplingParams::Uniform(TextureSamplingMode::Linear, texture.getLevels() > 1));
            },
            [&](const Texture2DArray& texture) {
                m_size = glm::ivec3 {texture.getSize()};
                glTexStorage3D_internal(target, static_cast<GLint>(texture.getLevels()), m_internalFormat, m_size.x, m_size.y, m_size.z, format);

                SetWrapMode(TextureWrapParams::Uniform(TextureWrapMode::Repeat));
                SetSamplingMode(TextureSamplingParams::Uniform(TextureSamplingMode::Linear, texture.getLevels() > 1));
            },
            [&](const Texture2DMultisampleArray& texture) {
                throw AT2TextureException("Not supported");
                // m_size = glm::ivec3 {texture.getSize()};
                // glTexStorage3DMultisample(target, static_cast<GLint>(texture.getSamples()), m_internalFormat,
                //                               m_size.x, m_size.y, m_size.z, texture.getFixedSampleLocations());

                // SetWrapMode(TextureWrapParams::Uniform(TextureWrapMode::Repeat));
                // SetSamplingMode(TextureSamplingParams::Uniform(TextureSamplingMode::Linear));
            },
            [&](const TextureCube& texture) {
                m_size = glm::ivec3 {texture.getSize(), 1};
                glTexStorage2D_internal(target, static_cast<GLint>(texture.getLevels()), m_internalFormat, m_size.x, m_size.y, format);

                SetWrapMode( TextureWrapParams::Uniform(TextureWrapMode::ClampToEdge));
                SetSamplingMode(TextureSamplingParams::Uniform(TextureSamplingMode::Linear, texture.getLevels() > 1));
            },
            [&](const TextureCubeArray& texture) {
                m_size = glm::ivec3 {texture.getSize()};
                glTexStorage3D_internal(target, static_cast<GLint>(texture.getLevels()), m_internalFormat, m_size.x, m_size.y,
                                   m_size.z, format);

                SetWrapMode(TextureWrapParams::Uniform(TextureWrapMode::ClampToEdge));
                SetSamplingMode(TextureSamplingParams::Uniform(TextureSamplingMode::Linear, texture.getLevels() > 1));
            },
            [&](const Texture3D& texture) {
                m_size = glm::ivec3 {texture.getSize()};
                glTexStorage3D_internal(target, static_cast<GLint>(texture.getLevels()), m_internalFormat, m_size.x, m_size.y,
                                   m_size.z, format);

                SetWrapMode(TextureWrapParams::Uniform(TextureWrapMode::Repeat));
                SetSamplingMode(TextureSamplingParams::Uniform(TextureSamplingMode::Linear, texture.getLevels() > 1));
            }},
        m_flavor);

    ReadChannelSizes();
}

GlTexture::~GlTexture()
{
    glDeleteTextures(1, &m_id);
}

GLenum GlTexture::Bind()
{
    m_stateManager.DoBind(shared_from_this());
    return GetTarget();
}

void GlTexture::BindAsImage(unsigned unit, glm::u32 level, glm::u32 layer, bool isLayered, BufferOperation usage) const
{
}

void GlTexture::BuildMipmaps()
{
    glGenerateMipmap(Bind());
}

void GlTexture::SetWrapMode(TextureWrapParams wrapParams)
{
    const auto target = Bind();
    
    m_wrapParams = wrapParams;

    glTexParameteri(target, GL_TEXTURE_WRAP_S, Mappings::TranslateWrapMode(wrapParams.WrapS));
    glTexParameteri(target, GL_TEXTURE_WRAP_T, Mappings::TranslateWrapMode(wrapParams.WrapT));
    glTexParameteri(target, GL_TEXTURE_WRAP_R, Mappings::TranslateWrapMode(wrapParams.WrapR));
}

void GlTexture::SetSamplingMode(TextureSamplingParams samplingParams)
{
    const auto target = Bind();
    
    m_sampling_params = samplingParams;

    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, Mappings::TranslateTextureSamplingModes(samplingParams.Magnification));
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, std::apply(Mappings::TranslateTextureSamplingModes, samplingParams.Minification));
}

void GlTexture::SetAnisotropy(float anisotropy)
{
    if (!GLAD_GL_ARB_texture_filter_anisotropic)
        return;


    glTexParameterf(Bind(), GL_TEXTURE_MAX_ANISOTROPY, m_anisotropy = std::min(anisotropy, max_anisotropy()));
}

float GlTexture::GetAnisotropy() const noexcept
{
    return m_anisotropy;
}

void GlTexture::SubImage1D(glm::u32 _offset, glm::u32 _size, glm::u32 _level, TextureFormat dataFormat, const void* data)
{
    const auto [externalFormat, externalType] = Mappings::TranslateTextureExternalFormatAndType(dataFormat);

    const auto offset = static_cast<GLint> (_offset);
    const auto size = static_cast<GLint> (_size);
    const auto level = static_cast<GLint>(_level);

    if (size + offset > m_size.x)
        throw AT2TextureException( "Some SubImage texels out of texture bounds");

    if (const auto* tex1D = std::get_if<Texture1D>(&GetType()))
    {
        if (size >= static_cast<int>(tex1D->getSize().x))
            throw AT2TextureException( "SubImage size more than texture actual size");

        glTexSubImage1D(Bind(), level, m_internalFormat, size, externalFormat, externalType, data);
    }
    else
        throw AT2NotImplementedException(
                           "SubImage1D operation could be performed only at Texture1D target");
}

void GlTexture::SubImage2D(glm::uvec2 _offset, glm::uvec2 _size, glm::u32 _level, TextureFormat dataFormat,
                           const void* data)
{
    const auto offset = glm::ivec2 {_offset};
    const auto size = glm::ivec2 {_size};
    const auto level = static_cast<GLint>(_level);

    if (const auto maxCoord = size + offset; maxCoord.x > m_size.x || maxCoord.y > m_size.y)
        throw AT2TextureException( "Some SubImage texels out of texture bounds");

    using namespace std;

    visit(
        [=, id=m_id]<typename T>(T) {
            if constexpr (is_same_v<T, Texture1DArray> || is_same_v<T, Texture2D>)
            {
                const auto [externalFormat, externalType] = Mappings::TranslateTextureExternalFormatAndType(dataFormat);
                glTexSubImage2D(Bind(), level, offset.x, offset.y, size.x, size.y, externalFormat,
                                       externalType, data);
            }
            else
                throw AT2NotImplementedException("SubImage2D supports only Texture1DArray, Texture2D, TextureCube");
        },
        GetType());
}

void GlTexture::SubImage3D(glm::uvec3 _offset, glm::uvec3 _size, glm::u32 _level, TextureFormat dataFormat,
                           const void* data)
{
    const auto offset = glm::ivec3{_offset};
    const auto size = glm::ivec3{_size};
    const auto level = static_cast<GLint>(_level);

    if (const auto maxCoord = size + offset;
        maxCoord.x > m_size.x || maxCoord.y > m_size.y || maxCoord.z > m_size.z)
        throw AT2TextureException( "Some SubImage texels out of texture bounds");

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

                const auto [externalFormat, externalType] = Mappings::TranslateTextureExternalFormatAndType(dataFormat);
                glTexSubImage3D(Bind(), level, offset.x, offset.y, offset.z, size.x, size.y, size.z, externalFormat,
                                    externalType, data);
            }
            else
                throw AT2NotImplementedException("SubImage3D supports only Texture2DArray, Texture3D");
        },
        GetType());
}

void GlTexture::CopyFromFramebuffer(GLint level, glm::ivec2 pos, glm::ivec2 size, glm::ivec3 textureOffset)
{
    if (const auto maxCoord = textureOffset + glm::ivec3(size, 1);
        maxCoord.x > m_size.x || maxCoord.y > m_size.y || maxCoord.z > m_size.z)
        throw AT2TextureException( "Some CopyFromFramebuffer texels out of texture bounds");

    using namespace std;

    visit(
        [=, id = m_id]<typename T>(T) {
            const auto target = Bind();
        
            if constexpr (is_same_v<T, Texture1DArray> || is_same_v<T, Texture2D> || is_same_v<T, Texture2DRectangle>)
                glCopyTexSubImage2D(target, level, textureOffset.x, textureOffset.y, pos.x, pos.y, size.x, size.y);
            else if constexpr (is_same_v<T, Texture3D> || is_same_v<T, Texture2DArray>)
                glCopyTexSubImage3D(target, level, textureOffset.x, textureOffset.y, textureOffset.z, pos.x, pos.y, size.x, size.y);
            else //is_same_v<T, TextureCubeArray> || is_same_v<T, TextureCube>
                throw AT2NotImplementedException( "Probably not implemented");
        },
        GetType());
}
