#ifndef AT2_BUFFER_MAPPER_GUARD_H
#define AT2_BUFFER_MAPPER_GUARD_H

#include "AT2.h"

namespace AT2
{
    class BufferMapperGuard
    {
    public:
        BufferMapperGuard(IBuffer& buffer, BufferOperation accessRights) :
            buffer(buffer), span(buffer.Map(accessRights)), access(accessRights)
        {
        }

        BufferMapperGuard(IBuffer& buffer, size_t offset, size_t length, BufferOperation accessRights) :
            buffer(buffer), span(buffer.MapRange(accessRights, offset, length)), access(accessRights)
        {
        }

        ~BufferMapperGuard()
        {
            buffer.Unmap();
        }

        NON_COPYABLE_OR_MOVABLE(BufferMapperGuard)

        const std::byte* data() const noexcept
        {
            assert(access.Contains(BufferOperationFlags::Read));
            return span.data();
        }

        std::byte* data() noexcept
        {
            assert(access.Contains(BufferOperationFlags::Write));
            return span.data();
        }

        template <typename T>
        T Get(size_t offset = 0) const
        {
            assert(offset + sizeof(T) <= span.size());
            assert(access.Contains(BufferOperationFlags::Read));

            T value;
            memcpy(&value, &span[offset], sizeof(T));
            return value;
        }

        template <typename T>
        void Set(const T& value, size_t offset = 0)
        {
            assert(offset + sizeof(T) <= span.size());
            assert(access.Contains(BufferOperationFlags::Write));

            memcpy(&span[offset], &value, sizeof(T));
        }

    private:
        IBuffer& buffer;
        std::span<std::byte> span;
        const BufferOperation access;
    };

} // namespace AT2

#endif
