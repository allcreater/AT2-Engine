#ifndef AT2_BUFFER_MAPPER_GUARD_H
#define AT2_BUFFER_MAPPER_GUARD_H

#include "AT2.h"

namespace AT2
{
    class BufferMapperGuard
    {
    public:
        BufferMapperGuard(IBuffer& buffer, BufferUsage accessRights) :
            buffer(buffer), begin(buffer.Map(accessRights)), length(buffer.GetLength()), access(accessRights)
        {
        }

        BufferMapperGuard(IBuffer& buffer, size_t offset, size_t length, BufferUsage accessRights) :
            buffer(buffer), begin(buffer.MapRange(accessRights, offset, length)), length(length), access(accessRights)
        {
        }

        ~BufferMapperGuard()
        {
            buffer.Unmap();
        }

        NON_COPYABLE_OR_MOVABLE(BufferMapperGuard)

        const std::byte* data() const noexcept
        {
            assert(static_cast<int>(access) | static_cast<int>(BufferUsage::Read));
            return begin;
        }

        std::byte* data() noexcept
        {
            assert(static_cast<int>(access) | static_cast<int>(BufferUsage::Write));
            return begin;
        }

        template <typename T>
        T Get(size_t offset = 0) const
        {
            assert(offset + sizeof(T) <= length);
            assert(static_cast<int>(access) | static_cast<int>(BufferUsage::Read));

            T value;
            memcpy(&value, begin + offset, sizeof(T));
            return value;
        }

        template <typename T>
        void Set(const T& value, size_t offset = 0)
        {
            assert(offset + sizeof(T) <= length);
            assert(static_cast<int>(access) | static_cast<int>(BufferUsage::Write));

            memcpy(begin + offset, &value, sizeof(T));
        }

    private:
        IBuffer& buffer;
        std::byte* const begin;
        const size_t length;
        const BufferUsage access;
    };

} // namespace AT2

#endif