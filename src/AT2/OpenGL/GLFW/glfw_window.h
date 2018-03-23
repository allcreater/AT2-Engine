#ifndef GLFW_WINDOW_HEADER
#define GLFW_WINDOW_HEADER

#include <iostream>
#include <functional>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#ifdef GLFW_WRAPPER_VIRTUAL_CALLBACKS
#define GLFW_WRAPPER_VIRTUAL virtual
#else
#define GLFW_WRAPPER_VIRTUAL 
#endif

struct GlfwException : public std::exception
{
    GlfwException(const char* reason) : std::exception(reason)
    {
    }
};

void InitGLFW();
void ReleaseGLFW();

class GlfwWindow
{
public:
    void Run();

    void setWindowLabel(const std::string& label);
    const std::string& getWindowLabel() const;

    void setWindowSize(int width, int height);

    //event callbacks
    std::function<void(void)> InitializeCallback;
    std::function<void(double, double)> RenderCallback;

    std::function<void(int)> KeyDownCallback;
    std::function<void(int)> KeyUpCallback;
    std::function<void(int)> KeyRepeatCallback;
    std::function<void(int, int)> ResizeCallback;
    std::function<void(void)> ClosingCallback;

    std::function<void(double, double)> MouseMoveCallback;
    std::function<void(int)> MouseDownCallback;
    std::function<void(int)> MouseUpCallback;

protected:
    GLFW_WRAPPER_VIRTUAL void OnInitialize() { if (InitializeCallback != nullptr) InitializeCallback(); }
    GLFW_WRAPPER_VIRTUAL void OnRender(double time, double deltaTime) { if (RenderCallback != nullptr) RenderCallback(time, deltaTime); }
    
    GLFW_WRAPPER_VIRTUAL void OnKeyDown(int key) const { if (KeyDownCallback != nullptr) KeyDownCallback(key); }
    GLFW_WRAPPER_VIRTUAL void OnKeyUp(int key) const { if (KeyUpCallback != nullptr) KeyUpCallback(key); }
    GLFW_WRAPPER_VIRTUAL void OnKeyRepeat(int key) const { if (KeyRepeatCallback != nullptr) KeyRepeatCallback(key); }
    
    GLFW_WRAPPER_VIRTUAL void OnResize(int newWidth, int newHeight) const { if (ResizeCallback != nullptr) ResizeCallback(newWidth, newHeight); }
    GLFW_WRAPPER_VIRTUAL void OnClosing() const { if (ClosingCallback != nullptr) ClosingCallback(); }
    
    GLFW_WRAPPER_VIRTUAL void OnMouseMove(double x, double y) const { if (MouseMoveCallback != nullptr) MouseMoveCallback(x, y); }
    GLFW_WRAPPER_VIRTUAL void OnMouseDown(int button) const { if (MouseDownCallback != nullptr) MouseDownCallback(button); }
    GLFW_WRAPPER_VIRTUAL void OnMouseUp(int button) const { if (MouseUpCallback != nullptr) MouseUpCallback(button); }

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
    int m_wndWidth = 800, m_wndHeight = 600;

    double m_previousRenderTime = 0.0;
};

#endif