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

class IFileSystemListener
{
public:
	typedef std::string namestring;
	typedef std::function<void(const namestring& filename)> callbackFunc;

	virtual void SetFolderCallback(const namestring& folderName, callbackFunc callback) = 0;
	virtual void ResetFolderCallback(const namestring& folderName) = 0;

public:
	static IFileSystemListener* GetInstance();

private:
	static IFileSystemListener* s_instance;
};

}