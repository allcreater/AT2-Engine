#pragma once

#include <AT2.h>

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

namespace AT2::Metal
{

template <typename T>
requires requires(T* t){ t->release(); }
struct MetalObjectReleaser
{
    void operator()(T* object) const noexcept
    {
        object->release();
    }
};

template <typename T>
using MtlPtr = std::unique_ptr<T, MetalObjectReleaser<T>>;

} //AT2::Metal
