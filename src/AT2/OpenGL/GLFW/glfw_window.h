#pragma once

#include <mutex>
#include <functional>

#include <GLFW/glfw3.h>

#include "callback_types.h"


enum class GlfwCursorMode
{
    Normal = GLFW_CURSOR_NORMAL,
    Hidden = GLFW_CURSOR_HIDDEN,
    Disabled = GLFW_CURSOR_DISABLED
};

enum class GlfwOpenglProfile
{
    Any = GLFW_OPENGL_ANY_PROFILE,
    Core = GLFW_OPENGL_CORE_PROFILE,
    Compat = GLFW_OPENGL_COMPAT_PROFILE
};

struct GlfwContextParameters
{
    GlfwOpenglProfile profile = GlfwOpenglProfile::Core;
    int context_major_version = 4;
    int context_minor_version = 3;

    int msaa_samples = GLFW_DONT_CARE;
    int refresh_rate = GLFW_DONT_CARE;

    bool srgb_capable = true;
    bool debug_context = false;
};


class GlfwWindow final
{
friend class GlfwApplication;

public:
    ///@thread_safety main thread
    bool isKeyDown(int keyCode) const;
    ///@thread_safety main thread
    bool isMouseKeyDown(int button) const;

    ///@thread_safety safe
    GlfwWindow& setCursorMode(GlfwCursorMode cursorMode);

    ///@thread_safety safe
    GlfwWindow& setLabel(const std::string& label);
    const std::string& getLabel() const { return window_label; }

    ///@thread_safety safe
    GlfwWindow& setSize(glm::ivec2 size);
    const glm::ivec2& getSize() const { return window_size; }

    ///@thread_safety safe
    void requestAttention();

    ///@thread_safety safe
    GlfwWindow& setVSyncInterval(int interval);

    ///@thread_safety safe
    GlfwWindow& setCloseFlag(bool flag);
    bool getCloseFlag() const;


    //event callbacks
    //Render context available only at InitializeCallback and RenderCallback

    std::function<void()> InitializeCallback {};
    std::function<void(double)> UpdateCallback {};
    std::function<void(double)> RenderCallback {};
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
    void OnInitialize() const { if (InitializeCallback) InitializeCallback(); }
    void OnUpdate(double deltaTime) const { if (UpdateCallback) UpdateCallback(deltaTime); }
    void OnRender(double deltaTime) const { if (RenderCallback) RenderCallback (deltaTime); }
    void OnWindowRefreshing() const { if (RefreshingCallback) RefreshingCallback(); }

    void OnKeyDown(int key) const { if (KeyDownCallback) KeyDownCallback(key); }

    void OnKeyUp(int key) const { if (KeyUpCallback) KeyUpCallback(key); }
    void OnKeyRepeat(int key) const { if (KeyRepeatCallback) KeyRepeatCallback(key); }
    
    void OnResize(glm::ivec2 newSize) const 
    { 
        if (ResizeCallback) 
            ResizeCallback(newSize); 

        window_size = newSize;
    }
    void OnClosing() const { if (ClosingCallback) ClosingCallback(); }
    
    void OnMouseMove(glm::dvec2 mousePosition) const 
    {
        mousePosition.y = window_size.y - mousePosition.y; //WTF?

        if (MouseMoveCallback != nullptr) 
            MouseMoveCallback(MousePos{ mousePosition, previous_mouse_pos });

        previous_mouse_pos = mousePosition;
    }
    void OnMouseDown(int button) const { if (MouseDownCallback) MouseDownCallback(button);}
    void OnMouseUp(int button) const { if (MouseUpCallback) MouseUpCallback(button); }
    void OnMouseScroll(const glm::dvec2& scrollDirection) const { if (MouseScrollCallback) MouseScrollCallback(scrollDirection); }

private:
    ///@thread_safety main thread
    GlfwWindow(GlfwContextParameters params);

    void SetupCallbacks();
    void MakeContextCurrent();

    ///@thread_safety main thread
    void Close();
    ///@thread_safety any thread
    void UpdateAndRender();

    static GlfwWindow* FromNativeWindow(const GLFWwindow* window);

private:
    const GlfwContextParameters context_parameters;
    GLFWwindow* window_impl { nullptr };

    std::string window_label{ "New window"};

    bool is_initialized { false };
    bool swap_interval_need_update { true };

    //mutables is just for track some parameters in callbacks
    mutable glm::ivec2 window_size { 800, 600 };
    mutable glm::vec2 previous_mouse_pos {};
    mutable int swap_interval{ 0 };

    mutable std::mutex mutex;

    double previous_render_time {0.0};
};
