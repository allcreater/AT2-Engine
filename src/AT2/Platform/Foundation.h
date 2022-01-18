#pragma once

#include <functional>
#include <mutex>
#include <variant>

#include "GraphicsContextInterface.h"

#include "callback_types.h"

namespace AT2
{
    class WindowBase : public IWindow
    {
    public:
        WindowBase(glm::ivec2 windowSize = {}, std::string label = {}) : window_label(std::move(label)), window_size {windowSize} {}
        //void* get() const noexcept { return window_impl; }

    	IPlatformGraphicsContext& getGraphicsContext() { return *graphicsContext; }
    	const IPlatformGraphicsContext& getGraphicsContext() const { return *graphicsContext; }

        void setWindowContext(std::unique_ptr<IWindowContext> newWindowContext) override { windowContext = std::move(newWindowContext); }

    	const std::string& getLabel() const final { return window_label; }
        glm::ivec2 getSize() const final { return window_size; }

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
        void OnInitialize() const;

        void OnUpdate(Seconds deltaTime) const;
        void OnRender(Seconds deltaTime) const;

        void OnWindowRefreshing() const;

        void OnKeyDown(int key) const;
        void OnKeyUp(int key) const;
        void OnKeyRepeat(int key) const;

        void OnResize(glm::ivec2 newSize) const;
        void OnClosing() const;

        void OnMouseMove(glm::dvec2 mousePosition) const;
        void OnMouseDown(int button) const;
        void OnMouseUp(int button) const;
        void OnMouseScroll(const glm::dvec2& scrollDirection) const;


        // Produces MouseMove event by relative mouse position
        void MoveMouse(const glm::vec2& mouseMove) const;
        void UpdateAndRender();

        virtual void PlatformClose() = 0;

        void Close();

    protected:
        std::string window_label {"New window"};

        //mutables is just for track some parameters in callbacks
        mutable glm::ivec2 window_size {0};
        mutable glm::vec2 previous_mouse_pos {};

        mutable std::mutex mutex;


        std::unique_ptr<IPlatformGraphicsContext> graphicsContext;
        std::unique_ptr<IWindowContext> windowContext;

    private:
        std::chrono::steady_clock::time_point previous_render_time;
        bool firstUpdate = true;
    };
} // namespace AT2::SDL
