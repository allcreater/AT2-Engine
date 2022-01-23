#pragma once

#include "AT2lowlevel.h"

namespace AT2::Metal
{
    class Renderer;

    class Buffer : public IBuffer
    {
    public:
        NON_COPYABLE_OR_MOVABLE(Buffer)

        Buffer(Renderer&, VertexBufferType bufferType);
        ~Buffer() override;

    public:
        //[[nodiscard]] VertexBufferType GetType() const noexcept { return type; }

        [[nodiscard]] size_t GetLength() const noexcept override;

        void SetDataRaw(std::span<const std::byte> data) override;
        void ReserveSpace(size_t size) override;

        std::span<std::byte> Map(BufferUsage usage) override;
        std::span<std::byte> MapRange(BufferUsage usage, size_t offset, size_t length) override;
        void Unmap() override;

        //for internal usage
        MTL::Buffer* getNativeHandle() { return buffer.get(); }
        
    private:
        Renderer& renderer;
        VertexBufferType type;

        MtlPtr<MTL::Buffer> buffer;
        
        bool m_mapped = false;
    };

} // namespace AT2::Metal
