#ifndef GLFW_WINDOW_HEADER
#define GLFW_WINDOW_HEADER

#include <iostream>
#include <functional>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "callback_types.h"

#ifdef GLFW_WRAPPER_VIRTUAL_CALLBACKS
#define GLFW_WRAPPER_VIRTUAL virtual
#else
#define GLFW_WRAPPER_VIRTUAL 
#endif

struct GlfwException : public std::runtime_error
{
    GlfwException(const char* reason) : std::runtime_error(reason)
    {
    }
};

void InitGLFW();
void ReleaseGLFW();

class GlfwWindow
{
public:
    void Run();

    //TODO: probably all this functions should be made thread-safe or made available only from callbacks(for example moved to some helper class)
    void setWindowLabel(const std::string& label);
    const std::string& getWindowLabel() const;

    void setWindowSize(int width, int height);
    const glm::ivec2& getWindowSize() const { return m_windowSize; }

    void setVSyncInterval(int interval);
    void setWindowCloseFlag(bool flag);

    //event callbacks
    std::function<void(void)> InitializeCallback;
    std::function<void(double, double)> RenderCallback;
    std::function<void(void)> RefreshingCallback;

    std::function<void(int)> KeyDownCallback;
    std::function<void(int)> KeyUpCallback;
    std::function<void(int)> KeyRepeatCallback;
    std::function<void(const glm::ivec2&)> ResizeCallback;
    std::function<void(void)> ClosingCallback;

    std::function<void(const MousePos&)> MouseMoveCallback;
    std::function<void(int)> MouseDownCallback;
    std::function<void(int)> MouseUpCallback;
    std::function<void(const glm::vec2)> MouseScrollCallback;

protected:
    GLFW_WRAPPER_VIRTUAL void OnInitialize() { if (InitializeCallback != nullptr) InitializeCallback(); }
    GLFW_WRAPPER_VIRTUAL void OnRender(double time, double deltaTime) { if (RenderCallback != nullptr) RenderCallback(time, deltaTime); }
    GLFW_WRAPPER_VIRTUAL void OnWindowRefreshing() const { if (RefreshingCallback != nullptr) RefreshingCallback(); }

    GLFW_WRAPPER_VIRTUAL void OnKeyDown(int key) const { if (KeyDownCallback != nullptr) KeyDownCallback(key); }
    GLFW_WRAPPER_VIRTUAL void OnKeyUp(int key) const { if (KeyUpCallback != nullptr) KeyUpCallback(key); }
    GLFW_WRAPPER_VIRTUAL void OnKeyRepeat(int key) const { if (KeyRepeatCallback != nullptr) KeyRepeatCallback(key); }
    
    GLFW_WRAPPER_VIRTUAL void OnResize(const glm::ivec2& newSize) const 
    { 
        if (ResizeCallback != nullptr) ResizeCallback(newSize); 

        m_windowSize = newSize;
    }
    GLFW_WRAPPER_VIRTUAL void OnClosing() const { if (ClosingCallback != nullptr) ClosingCallback(); }
    
    GLFW_WRAPPER_VIRTUAL void OnMouseMove(const glm::vec2& mousePosition) const 
    {
        if (MouseMoveCallback != nullptr) 
            MouseMoveCallback(MousePos(mousePosition, m_lastMousePos)); 

        m_lastMousePos = mousePosition;
    }
    GLFW_WRAPPER_VIRTUAL void OnMouseDown(int button) const { if (MouseDownCallback != nullptr) MouseDownCallback(button); }
    GLFW_WRAPPER_VIRTUAL void OnMouseUp(int button) const { if (MouseUpCallback != nullptr) MouseUpCallback(button); }
    GLFW_WRAPPER_VIRTUAL void OnMouseScroll(const glm::vec2& scrollDirection) const { if (MouseScrollCallback) MouseScrollCallback(scrollDirection); }

    void Render();

    static GlfwWindow* FromNativeWindow(const GLFWwindow* window);

    static void TryUpdateGlBindings();

private:
    void SetupCallbacks();
    void MakeContextCurrent();

private:
    const int OpenGL_Context_Version_Major = 4;
    const int OpenGL_Context_Version_Minor = 3;

private:
    GLFWwindow * m_window = nullptr;

    std::string m_windowLabel = "New window";

    //mutables is just for track some parameters in callbacks
    mutable glm::ivec2 m_windowSize = glm::ivec2(800, 600);
    mutable glm::vec2 m_lastMousePos;

    double m_previousRenderTime = 0.0;
};

#endif
