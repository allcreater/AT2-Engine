#pragma once

#include <functional>
#include <unordered_map>
#include <optional>

//TODO: include GSL?

namespace AT2::Utils
{

//as advised at https://stackoverflow.com/questions/45507041/how-to-check-if-weak-ptr-is-empty-non-assigned
template <typename T>
bool is_uninitialized(std::weak_ptr<T> const& weak) {
	using wt = std::weak_ptr<T>;
	return !weak.owner_before(wt{}) && !wt{}.owner_before(weak);
}


//TODO: is it lawful even at C+++20?
template <typename T>
auto reinterpret_span_cast(std::span<const std::byte> span)
{
    return std::span{reinterpret_cast<const T*>(span.data()), span.size() / sizeof(T)};
}
template <typename T>
auto reinterpret_span_cast(std::span<std::byte> span)
{
    return std::span{reinterpret_cast<T*>(span.data()), span.size() / sizeof(T)};
}

template <typename T, typename Fn>
auto make_unary_less(Fn&& transform)
{
    return[transform = std::forward<Fn>(transform)](const T& a, const T& b)
    {
        return transform(a) < transform(b);
    };
}

template <typename T, typename K, typename V = typename T::mapped_type>
requires requires(T map, const K& key) { map.find(key) != map.end(); }
const std::remove_pointer_t<V>* find(const T& map, const K& key)
{
    if (const auto it = map.find(key); it != map.end())
    {
        if constexpr (std::is_pointer_v<V>)
            return it->second;
        else
            return &it->second;
    }

    return nullptr;
 }

//overloaded trick
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...)->overloaded<Ts...>;

//idea from PVS-studio's presentation
template <typename Func>
class lazy
{
    using result_type = std::invoke_result_t<Func>;
    using optional_type = std::conditional_t<std::is_void_v<result_type>, bool, //if void
        std::optional<std::conditional_t<std::is_reference_v<result_type>, std::reference_wrapper<std::remove_reference_t<result_type>>, result_type>>
    >;

    Func initializer;
    optional_type value = {};

public:
    lazy(Func&& initializer) : initializer(std::forward<Func>(initializer)) {}

    lazy() = delete;
    lazy(const lazy&) = delete;
    lazy(lazy&&) = delete;
    lazy& operator= (const lazy&) = delete;
    lazy& operator= (lazy&&) = delete;
    ~lazy() = default;

    result_type operator ()()
    {
        if constexpr (std::is_void_v<result_type>)
        {
            if (!value)
            {
                initializer();
                value = true;
            }
        }
        else
        {
            if (!value)
                value = initializer();

            return *value;
        }
    }

    template <typename = std::enable_if_t<!std::is_void_v<result_type>>>
    operator result_type ()
    {
        return operator()();
    }

    void reset()
    {
        value = {};
    }

};

// from Guidline Support Library: https://github.com/microsoft/GSL
// final_action allows you to ensure something gets run at the end of a scope
template <class F>
class final_action
{
public:
    explicit final_action(F f) noexcept : f_(std::move(f)) {}

    final_action(final_action&& other) noexcept : f_(std::move(other.f_)), invoke_(std::exchange(other.invoke_, false)) {}

    final_action(const final_action&) = delete;
    final_action& operator=(const final_action&) = delete;
    final_action& operator=(final_action&&) = delete;

    ~final_action() noexcept
    {
        if (invoke_) f_();
    }

private:
    F f_;
    bool invoke_{ true };
};

struct string_hash
{
    using hash_type = std::hash<std::string_view>;
    using is_transparent = void;

    size_t operator()(const char* str) const { return hash_type {}(str); }
    size_t operator()(std::string_view str) const { return hash_type {}(str); }
    size_t operator()(std::string const& str) const { return hash_type {}(str); }
};

// does static_cast from pointer-like type to ref
// 1. debug-time dynamic cast (under assert)
// 2. run-time nullptr check (throws exception)
template <typename DestinationT, typename SourceT>
requires requires(SourceT ptr) { ptr == nullptr, static_cast<std::add_lvalue_reference_t<DestinationT>>(*ptr); }
std::add_lvalue_reference_t<DestinationT> safe_dereference_cast(SourceT ptr)
{
	if (ptr == nullptr)
	    throw std::bad_cast();

    using ResultT = std::add_lvalue_reference_t<DestinationT>;
    assert(dynamic_cast<std::add_pointer_t<ResultT>>(&*ptr));
    return static_cast<ResultT>(*ptr);
}

#ifdef __cpp_lib_generic_unordered_lookup
template<typename V>
using UnorderedStringMap = std::unordered_map<std::string, V, string_hash, std::equal_to<>>;
#else
} //namespace AT2::Utils
#include <map>
namespace AT2::Utils
{
template<typename V>
using UnorderedStringMap = std::map<std::string, V, std::less<>>;
#endif

}

#ifndef __cpp_lib_bind_front
namespace std
{
template<typename F, typename... FRONT_ARGS>
    auto bind_front(F&& f, FRONT_ARGS&&... front_args)
    {
        // front_args are copied because multiple invocations of this closure are possible
        return [captured_f = std::forward<F>(f), front_args...](auto&&... back_args) {
                   return std::invoke(captured_f, front_args...,
                                      std::forward<decltype(back_args)>(back_args)...);
               };
    }
}
#endif

#ifndef __cpp_lib_bit_cast
namespace std
{
template <class To, class From>
std::enable_if_t<
    sizeof(To) == sizeof(From) &&
    std::is_trivially_copyable_v<From> &&
    std::is_trivially_copyable_v<To>,
    To>
// constexpr support needs compiler magic
bit_cast(const From& src) noexcept
{
    static_assert(std::is_trivially_constructible_v<To>,
        "This implementation additionally requires destination type to be trivially constructible");
 
    To dst;
    std::memcpy(&dst, &src, sizeof(To));
    return dst;
}
}
#endif

#if defined( __cpp_lib_ranges) || _MSC_VER >= 1929
#include <ranges>
#else
//#include <iterator>
#include <span>
#include <algorithm>

namespace std::ranges
{
/*
template< class T >
auto begin( T&& t ) { return std::begin(t); }

template< class T >
auto end( T&& t ) { return std::end(t); }

template <class T>
using iterator_t = decltype(ranges::begin(std::declval<T&>()));

template< class T >
concept range = requires( T& t ) {
  ranges::begin(t); // equality-preserving for forward iterators
  ranges::end  (t);
};

template< class T >
  concept input_range =
    ranges::range<T> && std::input_iterator<ranges::iterator_t<T>>;

template< class T >
  concept forward_range =
    ranges::input_range<T> && std::forward_iterator<ranges::iterator_t<T>>;

template< class T >
  concept bidirectional_range =
    ranges::forward_range<T> && std::bidirectional_iterator<ranges::iterator_t<T>>;

template< class T >
  concept random_access_range =
    ranges::bidirectional_range<T> && std::random_access_iterator<ranges::iterator_t<T>>;

template< class T >
  concept contiguous_range =
    ranges::random_access_range<T> &&
    std::contiguous_iterator<ranges::iterator_t<T>> &&
    requires(T& t) {
      { ranges::data(t) } ->
        std::same_as<std::add_pointer_t<ranges::range_reference_t<T>>>;
    };
*/
template< class T >
  concept contiguous_range =
    requires(T& t) {
        std::span{t};
    };

template <typename R1, typename R2>
bool equal(R1&& range1, R2&& range2)
{
    return std::equal(std::begin(range1), std::end(range1), std::begin(range2));
}

template <typename R, typename It, typename F>
void transform(R&& range, It dst, F&& transformer)
{
    std::transform(std::begin(range), std::end(range), dst, std::forward<F>(transformer));
}

}
#endif
