#pragma once

#include <functional>
#include <mutex>
#include <variant>

#include <AT2_types.hpp>

#include "callback_types.h"

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
        virtual void* getPlatformSwapchain() const = 0;
        virtual void makeCurrent() = 0;
        virtual void swapBuffers() = 0;
        
    };

    class WindowBase : public IWindow
    {
    public:
        WindowBase(glm::ivec2 windowSize = {}, std::string label = {}) : window_size(windowSize), window_label(std::move(label)) {}
        //void* get() const noexcept { return window_impl; }

    	IPlatformGraphicsContext& getGraphicsContext() { return *graphicsContext; }
    	const IPlatformGraphicsContext& getGraphicsContext() const { return *graphicsContext; }

        void setWindowContext(std::unique_ptr<IWindowContext> newWindowContext) override { windowContext = std::move(newWindowContext); }

        //event callbacks
        //Render context available only at InitializeCallback and RenderCallback

        std::function<void()> InitializeCallback {};
        std::function<void(Seconds)> UpdateCallback {};
        std::function<void(Seconds)> RenderCallback {};
        std::function<void()> RefreshingCallback {};

        std::function<void(int)> KeyDownCallback {};
        std::function<void(int)> KeyUpCallback {};
        std::function<void(int)> KeyRepeatCallback {};
        std::function<void(const glm::ivec2&)> ResizeCallback {};
        std::function<void()> ClosingCallback {};

        std::function<void(const MousePos&)> MouseMoveCallback {};
        std::function<void(int)> MouseDownCallback {};
        std::function<void(int)> MouseUpCallback {};
        std::function<void(glm::dvec2)> MouseScrollCallback {};

    protected:
        void OnInitialize() const
        {
            if (InitializeCallback)
                InitializeCallback();
        }

        void OnUpdate(Seconds deltaTime) const
        {
            if (UpdateCallback)
                UpdateCallback(deltaTime);
        }

        void OnRender(Seconds deltaTime) const
        {
            if (RenderCallback)
                RenderCallback(deltaTime);
        }

        void OnWindowRefreshing() const
        {
            if (RefreshingCallback)
                RefreshingCallback();
        }

        void OnKeyDown(int key) const
        {
            if (KeyDownCallback)
                KeyDownCallback(key);
        }

        void OnKeyUp(int key) const
        {
            if (KeyUpCallback)
                KeyUpCallback(key);
        }

        void OnKeyRepeat(int key) const
        {
            if (KeyRepeatCallback)
                KeyRepeatCallback(key);
        }

        void OnResize(glm::ivec2 newSize) const
        {
            if (ResizeCallback)
                ResizeCallback(newSize);

            window_size = newSize;
        }
        void OnClosing() const
        {
            if (ClosingCallback)
                ClosingCallback();
        }

        void OnMouseMove(glm::dvec2 mousePosition) const
        {
            mousePosition.y = window_size.y - mousePosition.y; //WTF?

            if (MouseMoveCallback)
                MouseMoveCallback(MousePos {mousePosition, previous_mouse_pos});

            previous_mouse_pos = mousePosition;
        }

        void OnMouseDown(int button) const
        {
            if (MouseDownCallback)
                MouseDownCallback(button);
        }

        void OnMouseUp(int button) const
        {
            if (MouseUpCallback)
                MouseUpCallback(button);
        }

        void OnMouseScroll(const glm::dvec2& scrollDirection) const
        {
            if (MouseScrollCallback)
                MouseScrollCallback(scrollDirection);
        }

        // Produces MouseMove event by relative mouse position
        void MoveMouse(const glm::vec2& mouseMove) const
        {
            const auto mousePosition = previous_mouse_pos + mouseMove;

            if (MouseMoveCallback)
                MouseMoveCallback(MousePos {mousePosition, previous_mouse_pos});

            previous_mouse_pos = mousePosition;
        }

        void UpdateAndRender()
        {
            graphicsContext->makeCurrent();

            {
                if (firstUpdate)
                {
                    firstUpdate = false;
                    previous_render_time = std::chrono::steady_clock::now();

                    OnInitialize();
                    OnResize(getSize());
                }

                //TODO: encapsulate time in ITime
                const auto currentTime = std::chrono::steady_clock::now();
                OnUpdate(currentTime - previous_render_time);
                if (getSize().x > 0 && getSize().y > 0)
                    OnRender(currentTime - previous_render_time);

                previous_render_time = currentTime;
            }

            graphicsContext->swapBuffers();
        }

    protected:
        std::string window_label {"New window"};

        //mutables is just for track some parameters in callbacks
        mutable glm::ivec2 window_size {800, 600};
        mutable glm::vec2 previous_mouse_pos {};

        mutable std::mutex mutex;


        std::unique_ptr<IPlatformGraphicsContext> graphicsContext;
        std::unique_ptr<IWindowContext> windowContext;

    private:
        std::chrono::steady_clock::time_point previous_render_time;
        bool firstUpdate = true;
    };
} // namespace AT2::SDL
