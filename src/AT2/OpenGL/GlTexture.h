#ifndef AT2_GL_TEXTURE_H
#define AT2_GL_TEXTURE_H

#include "AT2lowlevel.h"
#include "../AT2_textures.hpp"

namespace AT2
{
    class GlTexture : public ITexture
    {
    public:
        NON_COPYABLE_OR_MOVABLE(GlTexture)

        GlTexture(Texture flavor, GLint internalFormat);
        ~GlTexture() override;

        void Bind(unsigned int module) const override;
        void BindAsImage(unsigned int module, glm::u32 level, glm::u32 layer, bool isLayered, BufferUsage usage = BufferUsage::ReadWrite) const override;

        void Unbind() override;
        void BuildMipmaps() override;
        glm::uvec3 GetSize() const noexcept override { return m_size; }
        size_t GetDataLength() const noexcept override { return m_dataSize; }

        int GetCurrentModule() const noexcept override { return m_currentTextureModule; }
        unsigned int GetId() const noexcept override { return m_id; }

        const Texture& GetType() const noexcept override { return m_flavor; }
        void SetWrapMode(TextureWrapMode wrapMode) override;
        const TextureWrapMode& GetWrapMode() const override { return m_wrapMode; }

        void SubImage1D(glm::u32 offset, glm::u32 size, glm::u32 level, ExternalTextureFormat dataFormat, void* data) override;
        void SubImage2D(glm::uvec2 offset, glm::uvec2 size, glm::u32 level, ExternalTextureFormat dataFormat, void* data, int cubeMapFace = 0) override;
        void SubImage3D(glm::uvec3 offset, glm::uvec3 size, glm::u32 level, ExternalTextureFormat dataFormat, void* data) override;

        void CopyFromFramebuffer(GLuint _level, glm::ivec2 pos, glm::uvec2 size, int cubeMapFace = 0);

        GLenum GetTarget() const;

    protected:
        void ReadChannelSizes();

    protected:
        Texture m_flavor;
        TextureWrapMode m_wrapMode;

        GLuint m_id{ 0 };
        mutable int m_currentTextureModule{ -1 };

        GLint m_internalFormat{0};
        glm::uvec3 m_size;

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

            [[nodiscard]] int InBytes() const noexcept { return (red + green + blue + alpha + depth + stencil + shared) / 8; }
        } m_channelSizes;
        size_t m_dataSize {0};
    };

}

#endif