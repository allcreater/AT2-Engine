#ifndef AT2_GL_VERTEXBUFFER_H
#define AT2_GL_VERTEXBUFFER_H

#include "AT2lowlevel.h"

namespace AT2
{

    class GlVertexBuffer : public IVertexBuffer
    {
    public:
        NON_COPYABLE_OR_MOVABLE(GlVertexBuffer)

        GlVertexBuffer(VertexBufferType bufferType);
        ~GlVertexBuffer() override;

    public:
        [[nodiscard]] unsigned int GetId() const noexcept override { return m_id; }
        [[nodiscard]] VertexBufferType GetType() const noexcept override { return m_publicType; }

        [[nodiscard]] size_t GetLength() const noexcept override { return m_length; }

        void Bind() override;
        void SetData(size_t length, const void* data) override;

        std::byte* Map(BufferUsage usage) override;
        std::byte* MapRange(BufferUsage usage, size_t offset, size_t length) override;
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

        enum class GlBufferUsageHint : GLenum
        {
            StreamDraw = GL_STREAM_DRAW,
            StreamRead = GL_STREAM_READ,
            StreamCopy = GL_STREAM_COPY,
            StaticDraw = GL_STATIC_DRAW,
            StaticRead = GL_STATIC_READ,
            StaticCopy = GL_STATIC_COPY,
            DynamicDraw = GL_DYNAMIC_DRAW,
            DynamicRead = GL_DYNAMIC_READ,
            DynamicCopy = GL_DYNAMIC_COPY
        };


    protected:
        GLuint m_id;
        size_t m_length {0};

        VertexBufferType m_publicType;
        GlBufferType m_privateType;
        GlBufferUsageHint m_usageHint = GlBufferUsageHint::StaticDraw;

        bool m_mapped = false;
    };

} // namespace AT2

#endif