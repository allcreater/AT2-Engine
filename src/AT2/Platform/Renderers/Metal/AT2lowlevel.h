#pragma once

#include <AT2.h>

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

namespace AT2::Metal
{

template<typename T>
//concept MetalObjectPtr = std::is_base_of_v<NS::Referencing<T>, T>;
concept MetalCppReferencable = requires (T* ptr)
{
    {ptr->retain()};// -> std::same_as<T*>;
    {ptr->release()};
    {ptr->autorelease()};// -> std::same_as<T*>;
    {ptr->retainCount()};// -> std::same_as<UInteger>;
};

template <MetalCppReferencable T>
class MtlPtr
{
public:
    using pointer = T*;
    using element_type = T;
    
    
    // special members
    
    MtlPtr() = default;
    MtlPtr(T* ptr) noexcept : ptr{ptr} { try_retain(); }

    MtlPtr(const MtlPtr& other) : ptr{other.ptr} { try_retain(); }
    MtlPtr(MtlPtr&& other) : ptr{std::exchange(other.ptr, nullptr)} {}

    MtlPtr& operator=(MtlPtr other) noexcept {
        swap(other);
        return *this;
    }
    
    ~MtlPtr() { try_release(); }
    
    
    // getters
    
    pointer get() const noexcept { return ptr; }
    pointer operator->() const noexcept { return ptr; }
    std::add_lvalue_reference_t<T> operator*() const noexcept { return *ptr; }
    
    size_t use_count() const noexcept { return ptr ? ptr->retainCount() : 0; }
    bool unique() const noexcept { return use_count() == 1; }
    operator bool() const noexcept { return ptr; }
    
    
    // setters
    
    void reset(T* newPtr = nullptr) noexcept { MtlPtr{newPtr}.swap(*this); }
    void swap(MtlPtr& other) noexcept { std::swap(ptr, other.ptr); }
    
private:
    void try_retain()
    {
        if (ptr)
            ptr->retain();
    }
    
    void try_release()
    {
        if (ptr)
            ptr->release();
    }
    
private:
    pointer ptr = nullptr;
};

template <typename T>
auto ConstructMetalObject()
{
    return MtlPtr<T>{T::alloc()->init()};
}

} //AT2::Metal
