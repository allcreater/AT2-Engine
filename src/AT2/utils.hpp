#pragma once

#include <functional>
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

template<typename V>
using UnorderedStringMap = std::unordered_map<std::string, V, string_hash, std::equal_to<>>;

}