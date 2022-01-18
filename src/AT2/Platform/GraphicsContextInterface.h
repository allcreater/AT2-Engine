#pragma once

#include <AT2_types.hpp>

namespace AT2
{
    //TODO: move to separate file
    enum class OpenglProfile
    {
        Core,
        Compat
    };

    struct OpenGLContextParams
    {
        OpenglProfile profile = OpenglProfile::Core;
        int context_major_version = 4;
        int context_minor_version = 5;

        bool debug_context = false;
    };

    struct MetalContextParams
    {
    };

    using ConcreteContextParams = std::variant<std::monostate, OpenGLContextParams, MetalContextParams>;

    struct ContextParameters
    {
        ConcreteContextParams contextType;

        int msaa_samples = 0;
        int refresh_rate = 0;

        int framebuffer_bits_red = 8;
        int framebuffer_bits_green = 8;
        int framebuffer_bits_blue = 8;
        int framebuffer_bits_depth = 24;

        bool srgb_capable = true;
    };

    struct IPlatformGraphicsContext
    {
        virtual ~IPlatformGraphicsContext() = default;

        virtual void setVSyncInterval(int interval) = 0;
        [[nodiscard]] virtual void* getPlatformSwapchain() const = 0;
        [[nodiscard]] virtual glm::ivec2 getPhysicalViewportSize() const = 0;
        virtual void makeCurrent() = 0;
        virtual void swapBuffers() = 0;
    };

} // namespace AT2