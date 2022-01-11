#pragma once

#include "graphics_context.h"

namespace AT2::SDL
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
        const std::string& getLabel() const override { return window_label; }

        ///@thread_safety safe
        Window& setSize(glm::ivec2 size) override;
        const glm::ivec2& getSize() const override { return window_size; }

        ///@thread_safety safe
        void requestAttention() override;

        ///@thread_safety safe
        Window& setVSyncInterval(int interval) override;

        ///@thread_safety safe
        Window& setCloseFlag(bool flag) override { closeFlag = flag; return *this; }
        bool getCloseFlag() const override { return closeFlag; }

        SDL_Window* get() const noexcept { return window_impl; }
        
    private:
        ///@thread_safety main thread
        Window(const ContextParameters& contextParameters, glm::ivec2 initialSize);

        ///@thread_safety main thread
        void PlatformClose() override;

        ///@thread_safety any thread
        static Window* FromNativeWindow(SDL_Window* window);

    private:
        SDL_Window* window_impl {nullptr};
        std::atomic<bool> closeFlag{false};
    };
} //AT2::SDL
