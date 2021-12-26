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

    private:
        size_t m_length {0};

        VertexBufferType m_publicType;

        bool m_mapped = false;
    };

} // namespace AT2::Metal
