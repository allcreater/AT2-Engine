#pragma once

#include "AT2.h"

namespace AT2
{
    class ArrayBuffer : public IBuffer
    {
    public:
        ArrayBuffer() = default;

    public:
        [[nodiscard]] virtual size_t GetLength() const noexcept override { return m_data.size(); }
        virtual void SetDataRaw(std::span<const std::byte> data) override { m_data.assign(data.begin(), data.end()); }
        virtual void ReserveSpace(size_t size) override { if (m_data.size() != size) m_data.resize(size); }

        virtual std::span<std::byte> Map(BufferOperation usage) override
        {
            assert(!m_is_locked);
            m_is_locked = true;
            return m_data;
        }
        
        virtual std::span<std::byte> MapRange(BufferOperation usage, size_t offset, size_t length) override { return Map(usage).subspan(offset).first(length); };
        virtual void Unmap() override { assert(m_is_locked); m_is_locked = false; }
        
    private:
        std::vector<std::byte> m_data;
        bool m_is_locked = false;
    };

}
