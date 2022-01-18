#include "window.h"
#include <cassert>
#include <mutex>

#include "application.h"

using namespace AT2;
using namespace AT2::SDL;

namespace
{
	constexpr char windowDataKey_this[] = "AT2_this_window";
}


Window* Window::FromNativeWindow(SDL_Window* window)
{
    auto* const frontendPtr = static_cast<Window*>(SDL_GetWindowData(window, windowDataKey_this));
    assert(frontendPtr);
    return frontendPtr;
}

Window::Window(const ContextParameters& contextParameters, glm::ivec2 initialSize)
	: WindowBase{ initialSize}
{
    const auto windowFlags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | GetContextSpecificWindowFlags(contextParameters);
    window_impl = SDL_CreateWindow(window_label.c_str(), 100, 100, window_size.x, window_size.y, windowFlags);
    if (!window_impl)
        throw Exception("Window creation failed");

    graphicsContext = MakeGraphicsContext(window_impl, contextParameters);

    SDL_SetWindowData(window_impl, windowDataKey_this, this);
}

bool Window::isKeyDown(int keyCode) const
{
    return window_impl && SDL_GetKeyboardState(nullptr)[keyCode];
}

bool Window::isMouseKeyDown(int button) const
{
    glm::ivec2 pos;
    return window_impl && (SDL_GetMouseState(&pos.x, &pos.y) & SDL_BUTTON(button - 1));
}

Window& Window::setCursorMode(CursorMode cursorMode)
{
    ConcreteApplication::get().postAction([=] {
        if (!window_impl)
            return;

        SDL_ShowCursor(cursorMode == CursorMode::Normal ? SDL_ENABLE : SDL_DISABLE);

        const auto relativeMode = static_cast<SDL_bool>(cursorMode == CursorMode::Disabled);
        
        //SDL_SetWindowGrab(window_impl, relativeMode);
        SDL_SetRelativeMouseMode(relativeMode);
    });

    return *this;
}

Window& Window::setLabel(std::string label)
{
    {
        std::lock_guard lock {mutex};
        window_label = std::move(label);
    }

    ConcreteApplication::get().postAction([this] {
        if (window_impl != nullptr)
            SDL_SetWindowTitle(window_impl, window_label.c_str());
    });

    return *this;
}

//TODO: move to Foundation when application task query will be available from everywhere
Window& Window::setVSyncInterval(int interval)
{
    ConcreteApplication::get().postAction([this, interval] {
    	graphicsContext->makeCurrent();
        graphicsContext->setVSyncInterval(interval);
    });

    return *this;
}

void Window::requestAttention()
{
    ConcreteApplication::get().postAction([=] {
        if (window_impl)
            SDL_FlashWindow(window_impl, SDL_FLASH_UNTIL_FOCUSED);
    });
}

Window& Window::setSize(glm::ivec2 size)
{
    {
        std::lock_guard lock {mutex};
        window_size = size;
    }

    ConcreteApplication::get().postAction([=] {
        if (window_impl != nullptr)
            SDL_SetWindowSize(window_impl, size.x, size.y);
    });

    return *this;
}

void Window::PlatformClose()
{
    std::lock_guard lock {mutex};

    assert(window_impl);

    SDL_DestroyWindow(std::exchange(window_impl, nullptr));

}
