#include "Foundation.h"

using namespace AT2;

void AT2::WindowBase::OnInitialize() const
{
    if (InitializeCallback)
        InitializeCallback();
}

void AT2::WindowBase::OnUpdate(Seconds deltaTime) const
{
    if (UpdateCallback)
        UpdateCallback(deltaTime);
}

void AT2::WindowBase::OnRender(Seconds deltaTime) const
{
    if (RenderCallback)
        RenderCallback(deltaTime);
}

void AT2::WindowBase::OnWindowRefreshing() const
{
    if (RefreshingCallback)
        RefreshingCallback();
}

void AT2::WindowBase::OnKeyDown(int key) const
{
    if (KeyDownCallback)
        KeyDownCallback(key);
}

void AT2::WindowBase::OnKeyUp(int key) const
{
    if (KeyUpCallback)
        KeyUpCallback(key);
}

void AT2::WindowBase::OnKeyRepeat(int key) const
{
    if (KeyRepeatCallback)
        KeyRepeatCallback(key);
}

void AT2::WindowBase::OnResize(glm::ivec2 newSize) const
{
    assert(newSize.x > 0 && newSize.y > 0);

    if (ResizeCallback)
        ResizeCallback(newSize);

    window_size = newSize;
}

void AT2::WindowBase::OnClosing() const
{
    if (ClosingCallback)
        ClosingCallback();
}

void AT2::WindowBase::OnMouseMove(glm::dvec2 mousePosition) const
{
    mousePosition.y = window_size.y - mousePosition.y; //WTF?

    if (MouseMoveCallback)
        MouseMoveCallback(MousePos {mousePosition, previous_mouse_pos});

    previous_mouse_pos = mousePosition;
}

void AT2::WindowBase::OnMouseDown(int button) const
{
    if (MouseDownCallback)
        MouseDownCallback(button);
}

void AT2::WindowBase::OnMouseUp(int button) const
{
    if (MouseUpCallback)
        MouseUpCallback(button);
}

void AT2::WindowBase::OnMouseScroll(const glm::dvec2& scrollDirection) const
{
    if (MouseScrollCallback)
        MouseScrollCallback(scrollDirection);
}

void AT2::WindowBase::MoveMouse(const glm::vec2& mouseMove) const
{
    const auto mousePosition = previous_mouse_pos + mouseMove;

    if (MouseMoveCallback)
        MouseMoveCallback(MousePos {mousePosition, previous_mouse_pos});

    previous_mouse_pos = mousePosition;
}

void AT2::WindowBase::UpdateAndRender()
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

void AT2::WindowBase::Close()
{
    graphicsContext->makeCurrent();

    windowContext.reset();
    graphicsContext.reset();

    PlatformClose();
}
