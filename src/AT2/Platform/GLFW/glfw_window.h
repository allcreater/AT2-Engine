#pragma once

#include "../Foundation.h"

#include <GLFW/glfw3.h>

namespace AT2::GLFW
{
    class Window final : public WindowBase
    {
        friend class ConcreteApplication;

    public:
        ///@thread_safety main thread
        bool isKeyDown(int keyCode) const override;
        ///@thread_safety main thread
        bool isMouseKeyDown(int button) const override;

        ///@thread_safety safe
        Window& setCursorMode(CursorMode cursorMode) override;

        ///@thread_safety safe
        Window& setLabel(std::string label) override;

        ///@thread_safety safe
        Window& setSize(glm::ivec2 size) override;

        ///@thread_safety safe
        void requestAttention() override;

        ///@thread_safety safe
        Window& setVSyncInterval(int interval) override;

        ///@thread_safety safe
        Window& setCloseFlag(bool flag) override;
        bool getCloseFlag() const override;

        GLFWwindow* get() const noexcept { return window_impl; }

    private:
        ///@thread_safety main thread
        Window(ContextParameters params, glm::ivec2 initialSize, GLFWmonitor* monitor = nullptr);

        void SetupCallbacks();
        void MakeContextCurrent();

        ///@thread_safety main thread
        void PlatformClose() override;

        static Window* FromNativeWindow(const GLFWwindow* window);

    private:
        GLFWwindow* window_impl {nullptr};

        bool swap_interval_need_update {true};

        //mutables is just for track some parameters in callbacks
        mutable int swap_interval {0};
    };

} // namespace AT2::GLFW
