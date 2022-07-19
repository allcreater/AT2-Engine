#ifndef AT2_GL_TEXTURE_H
#define AT2_GL_TEXTURE_H

#include "../AT2_textures.hpp"
#include "AT2lowlevel.h"

namespace AT2::OpenGL
{
    class GlTexture : public ITexture
    {
    public:
        NON_COPYABLE_OR_MOVABLE(GlTexture)

        GlTexture(Texture flavor, GLint internalFormat);
        ~GlTexture() override;

        void BindAsImage(unsigned int unit, glm::u32 level, glm::u32 layer, bool isLayered,
                         BufferOperation usage = BufferOperationFlags::ReadWrite) const override;

        void BuildMipmaps() override;
        glm::uvec3 GetSize() const noexcept override { return glm::uvec3(m_size); }
        size_t GetDataLength() const noexcept override { return m_dataSize; }

        unsigned int GetId() const noexcept { return m_id; }

        const Texture& GetType() const noexcept override { return m_flavor; }

        void SubImage1D(glm::u32 offset, glm::u32 size, glm::u32 level, ExternalTextureFormat dataFormat,
                        const void* data) override;
        void SubImage2D(glm::uvec2 offset, glm::uvec2 size, glm::u32 level, ExternalTextureFormat dataFormat,
                        const void* data) override;
        void SubImage3D(glm::uvec3 offset, glm::uvec3 size, glm::u32 level, ExternalTextureFormat dataFormat,
                        const void* data) override;

        void CopyFromFramebuffer(int _level, glm::ivec2 pos, glm::ivec2 size, glm::ivec3 textureOffset = {});

        GLenum GetTarget() const;

        // ISampler implementation:
        void SetWrapMode(TextureWrapParams wrapParams) override;
        const TextureWrapParams& GetWrapMode() const noexcept override { return m_wrapParams; }

        void SetSamplingMode(TextureSamplingParams samplingParams) override;
        [[nodiscard]] const TextureSamplingParams& GetSamplingParams() const noexcept override { return m_sampling_params; }

        void SetAnisotropy(float anisotropy) override;
        [[nodiscard]] float GetAnisotropy() const noexcept override;

    protected:
        void ReadChannelSizes();

    protected:
        Texture m_flavor;
        TextureWrapParams m_wrapParams;
        TextureSamplingParams m_sampling_params;
        float m_anisotropy = 1.0f;

        GLuint m_id {0};
        GLenum m_internalFormat {0};
        //internal size representation is int instead of uint, so that we need to do casting on request :(
        glm::ivec3 m_size;

        //Not so needed to be persistent, but let it stay for info
        struct ChannelSize
        {
            GLint red;
            GLint green;
            GLint blue;
            GLint alpha;
            GLint depth;
            GLint stencil;
            GLint shared;

            [[nodiscard]] int InBytes() const noexcept
            {
                return (red + green + blue + alpha + depth + stencil + shared) / 8;
            }
        } m_channelSizes;
        size_t m_dataSize {0};
    };

} // namespace AT2::OpenGL

#endif
