#pragma once

#include <bit>
#include <compare>

namespace AT2
{
    template <typename Enum> concept FlaggedEnum = std::is_enum_v<Enum> && std::is_unsigned_v<std::underlying_type_t<Enum>>;

    template <FlaggedEnum Enum>
    class EnumFlags
    {
    public:
        using FlagType = Enum;
        using UnderlyingType = std::underlying_type_t<Enum>;
        constexpr static size_t NumberOfBits = std::numeric_limits<UnderlyingType>::digits;

        constexpr EnumFlags() : m_flags{0} {}
        constexpr EnumFlags(Enum flag) : m_flags{static_cast<UnderlyingType>(flag)} {}
        constexpr explicit EnumFlags(UnderlyingType flags) : m_flags {flags} {}

        template<typename... Flags>
        requires (std::is_same_v<Flags, Enum> && ...)
        constexpr EnumFlags(Flags ... flags) : m_flags((static_cast<UnderlyingType>(flags) | ...)) {}

        constexpr bool Any() const noexcept { return m_flags != 0; }
        constexpr bool None() const noexcept { return m_flags == 0; }
        constexpr bool JustOne() const noexcept { return m_flags && ((m_flags & (m_flags - 1)) == 0); } //std::has_single_bit
        constexpr int Count() const noexcept { return std::popcount(m_flags); }
        constexpr bool Contains(EnumFlags requiredFlags) const noexcept { return (m_flags & requiredFlags.m_flags) == requiredFlags.m_flags; }

        constexpr explicit operator Enum() const noexcept { return static_cast<Enum>(m_flags); }

        constexpr auto operator<=>(const EnumFlags& other) const noexcept = default;
    
        EnumFlags& operator |= (EnumFlags flags) { m_flags |= flags.m_flags; return *this;}
        EnumFlags& operator &= (EnumFlags flags) { m_flags &= flags.m_flags; return *this;}
        EnumFlags& operator ^= (EnumFlags flags) { m_flags ^= flags.m_flags; return *this;}
        constexpr EnumFlags operator~() const noexcept { return EnumFlags {~m_flags}; }

        friend constexpr EnumFlags operator| (EnumFlags lhv, EnumFlags rhv) noexcept { return lhv |= rhv; }
        friend constexpr EnumFlags operator& (EnumFlags lhv, EnumFlags rhv) noexcept { return lhv &= rhv; }
        friend constexpr EnumFlags operator^ (EnumFlags lhv, EnumFlags rhv) noexcept { return lhv ^= rhv; }

    private:
        UnderlyingType m_flags;
    };

    //TODO: write unit tests
}
