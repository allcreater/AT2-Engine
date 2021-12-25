#pragma once

#include "AT2lowlevel.h"

namespace AT2::Metal
{
    class VertexBuffer : public IVertexBuffer
    {
    public:
        NON_COPYABLE_OR_MOVABLE(VertexBuffer)

        VertexBuffer(VertexBufferType bufferType);
        ~VertexBuffer() override;

    public:
        [[nodiscard]] unsigned int GetId() const noexcept override { return 0; }
        [[nodiscard]] VertexBufferType GetType() const noexcept override { return m_publicType; }

        [[nodiscard]] size_t GetLength() const noexcept override { return m_length; }

        void Bind() override;
        void SetDataRaw(std::span<const std::byte> data) override;

        std::span<std::byte> Map(BufferUsage usage) override;
        std::span<std::byte> MapRange(BufferUsage usage, size_t offset, size_t length) override;
        void Unmap() override;

    protected:
        enum class BufferType : GLenum //TODO: properly handle all of them :)
        {
            ArrayBuffer = 0,
            AtomicCounterBuffer = 0,
            CopyReadBuffer = 0,
            CopyWriteBuffer = 0,
            DispatchIndirectBuffer = 0,
            DrawIndirectBuffer = 0,
            ElementArrayBuffer = 0,
            PixelPackBuffer = 0,
            PixelUnpackBuffer = 0,
            QueryBuffer = 0,
            ShaderStorageBuffer = 0,
            TextureBuffer = 0,
            TransformFeedbackBuffer = 0,
            UniformBuffer = 0
        };


    private:
        size_t m_length {0};

        VertexBufferType m_publicType;
        BufferType m_privateType;

        bool m_mapped = false;
    };

} // namespace AT2::Metal