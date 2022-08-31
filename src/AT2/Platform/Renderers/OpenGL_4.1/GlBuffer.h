#ifndef AT2_GL_VERTEXBUFFER_H
#define AT2_GL_VERTEXBUFFER_H

#include "AT2lowlevel.h"

namespace AT2::OpenGL41
{
    class GlBuffer : public IBuffer
    {
    public:
        NON_COPYABLE_OR_MOVABLE(GlBuffer)

        GlBuffer(VertexBufferType bufferType);
        ~GlBuffer() override;

    public:
        [[nodiscard]] unsigned int GetId() const noexcept { return m_id; }
        [[nodiscard]] VertexBufferType GetType() const noexcept { return m_publicType; } //TODO: don't use, binding type shoud be deduced from usage

        [[nodiscard]] size_t GetLength() const noexcept override { return m_length; }

        void SetDataRaw(std::span<const std::byte> data) override;
        void ReserveSpace(size_t size) override;

        std::span<std::byte> Map(BufferOperation usage) override;
        std::span<std::byte> MapRange(BufferOperation usage, size_t offset, size_t length) override;
        void Unmap() override;

    protected:
        enum class GlBufferType : GLenum //TODO: properly handle all of them :)
        {
            ArrayBuffer = GL_ARRAY_BUFFER,
            AtomicCounterBuffer = GL_ATOMIC_COUNTER_BUFFER,
            CopyReadBuffer = GL_COPY_READ_BUFFER,
            CopyWriteBuffer = GL_COPY_WRITE_BUFFER,
            DispatchIndirectBuffer = GL_DISPATCH_INDIRECT_BUFFER,
            DrawIndirectBuffer = GL_DRAW_INDIRECT_BUFFER,
            ElementArrayBuffer = GL_ELEMENT_ARRAY_BUFFER,
            PixelPackBuffer = GL_PIXEL_PACK_BUFFER,
            PixelUnpackBuffer = GL_PIXEL_UNPACK_BUFFER,
            QueryBuffer = GL_QUERY_BUFFER,
            ShaderStorageBuffer = GL_SHADER_STORAGE_BUFFER,
            TextureBuffer = GL_TEXTURE_BUFFER,
            TransformFeedbackBuffer = GL_TRANSFORM_FEEDBACK_BUFFER,
            UniformBuffer = GL_UNIFORM_BUFFER
        };

    private:
        GLuint m_id;
        size_t m_length {0};

        VertexBufferType m_publicType;

        bool m_mapped = false;
    };

} // namespace AT2::OpenGL41

#endif