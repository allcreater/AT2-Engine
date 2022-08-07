#pragma once

#include "AT2lowlevel.h"

namespace AT2::Metal
{
    class Renderer;

    class Buffer : public IBuffer
    {
    public:
        NON_COPYABLE_OR_MOVABLE(Buffer)

        Buffer(Renderer& renderer, VertexBufferType bufferType);
        ~Buffer() override;

    public:
        [[nodiscard]] size_t GetLength() const noexcept override;

        void SetDataRaw(std::span<const std::byte> data) override;
        void ReserveSpace(size_t size) override;

        std::span<std::byte> Map(BufferOperation usage) override;
        std::span<std::byte> MapRange(BufferOperation usage, size_t offset, size_t length) override;
        void Unmap() override;

        //for internal usage
        MTL::Buffer* getNativeHandle() { return m_buffer.get(); }
        
    private:
        Renderer& m_renderer;
        VertexBufferType m_type;

        MtlPtr<MTL::Buffer> m_buffer;
        
        std::optional<NS::Range> m_mappedRange;
    };

} // namespace AT2::Metal
