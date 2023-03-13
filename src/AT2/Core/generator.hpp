#pragma once

#ifdef __cpp_lib_coroutine
#include <coroutine>
namespace coroutines = std;
using range_santinel = std::default_sentinel_t;
#else
#include <experimental/coroutine>
namespace coroutines = std::experimental;
struct range_santinel {};
#endif 


namespace detail
{

}

template <typename T>
class [[nodiscard]] generator
{
    struct promise_type
    {
        T current_value;
        std::exception_ptr exception;

        coroutines::suspend_always initial_suspend() noexcept { return {}; }
        coroutines::suspend_always final_suspend() noexcept { return {}; }
        coroutines::suspend_always yield_value(T value) noexcept
        {
            current_value = std::move(value);
            return {};
        }
        void return_void() {}
        generator get_return_object() { return generator{ coroutines::coroutine_handle<promise_type>::from_promise(*this) }; }
        void unhandled_exception() { exception = std::current_exception(); }

        void rethrow_if_exception()
        {
            if (exception)
                std::rethrow_exception(exception);
        }
    };

    friend struct promise_type;
    explicit generator(coroutines::coroutine_handle<promise_type> coroutine_handle) : coroutine_handle(coroutine_handle) {}

public:
    using promise_type = promise_type;
    using handle_type = coroutines::coroutine_handle<promise_type>;

    generator() = default;
    generator(const generator& other) = delete;
    generator& operator=(const generator& other) = delete;

    generator(generator&& other) noexcept : coroutine_handle(std::exchange(other.coroutine_handle, {})) {}
    ~generator()
    { 
        if (coroutine_handle) coroutine_handle.destroy();
    }

    generator& operator=(generator&& other) noexcept
    {
        if (this != &other)
        {
            if (coroutine_handle) coroutine_handle.destroy();
            coroutine_handle = std::exchange(other.coroutine_handle, {});
        }
        return *this;
    }

    class iterator
    {
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        iterator() = default;
        explicit iterator(handle_type coro) : coroutine_handle(coro) {}

        iterator& operator++()
        {
            coroutine_handle.resume();
            if (coroutine_handle.done())
                coroutine_handle.promise().rethrow_if_exception();

            return *this;
        }

        iterator operator++(int)
        {
            auto tmp = *this;
            ++*this;
            return tmp;
        }

        T& operator*() const { return coroutine_handle.promise().current_value; }
        T* operator->() const { return std::addressof(operator*()); }

        bool operator==(const range_santinel&) const { return coroutine_handle.done(); }

    private:
        handle_type coroutine_handle;
    };

    iterator begin()
    {
        if (coroutine_handle)
            coroutine_handle.resume();

        return iterator{ coroutine_handle };
    }

    range_santinel end() { return {}; }

private:
    handle_type coroutine_handle;
};