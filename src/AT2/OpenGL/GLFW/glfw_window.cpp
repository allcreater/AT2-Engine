#include "glfw_window.h"
#include <mutex>
#include <cassert>


void InitGLFW()
{
    if (!glfwInit())
        throw new GlfwException("Initialization failed");

    glfwSetErrorCallback([](int, const char* message) {
        throw new GlfwException(message);
    });
}

void ReleaseGLFW()
{
    glfwTerminate();
}


GlfwWindow* GlfwWindow::FromNativeWindow(const GLFWwindow* window)
{
    return reinterpret_cast<GlfwWindow*>(glfwGetWindowUserPointer(const_cast<GLFWwindow*>(window)));
}

void GlfwWindow::TryUpdateGlBindings()
{
    //Unfortunately Huter package don't contains MX version so it seems we must reset it after each context switching
    //TODO: switch to Glew MX

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
		throw new GlfwException("Failed to initialize GLEW"); //yes, it's strange to throw a Glfw exception :3
}

void GlfwWindow::Run()
{
    static std::mutex m;

    {
        std::lock_guard<std::mutex> lock(m);

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OpenGL_Context_Version_Major);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OpenGL_Context_Version_Minor);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_SAMPLES, 8);

        /* Create a windowed mode window and its OpenGL context */
        m_window = glfwCreateWindow(m_wndWidth, m_wndHeight, m_windowLabel.c_str(), nullptr, nullptr);
        if (!m_window)
            throw new GlfwException("Windows creation failed");

        glfwSetWindowUserPointer(m_window, this);

        SetupCallbacks();
    }

    MakeContextCurrent();
    OnInitialize();

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(m_window))
    {
        MakeContextCurrent();

        {
            double currentTime = glfwGetTime();
            OnRender(currentTime, currentTime - m_previousRenderTime);
            m_previousRenderTime = currentTime;
        }

        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }

    OnClosing();
}

void GlfwWindow::setWindowLabel(const std::string & label)
{
    m_windowLabel = label;
    if (m_window != nullptr)
        glfwSetWindowTitle(m_window, label.c_str());
}

const std::string & GlfwWindow::getWindowLabel() const
{
    return m_windowLabel;
}

void GlfwWindow::setWindowSize(int width, int height)
{
    if (m_window != nullptr)
        glfwSetWindowSize(m_window, m_wndWidth, m_wndHeight);

    m_wndWidth = width;
    m_wndHeight = height;
}

void GlfwWindow::SetupCallbacks()
{

    glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto wnd = GlfwWindow::FromNativeWindow(window);

        switch (action)
        {
        case GLFW_RELEASE:
            wnd->OnKeyUp(key); break;
        case GLFW_PRESS:
            wnd->OnKeyDown(key); break;
        case GLFW_REPEAT:
            wnd->OnKeyRepeat(key); break;
        default:
            throw new GlfwException("Unknown key action");
        }
        
    });

    glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int mods) {
        auto wnd = GlfwWindow::FromNativeWindow(window);
        if (action == GLFW_PRESS)
            wnd->OnMouseDown(button);
        else if (action == GLFW_RELEASE)
            wnd->OnMouseUp(button);
    });

    glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double x, double y) {
        GlfwWindow::FromNativeWindow(window)->OnMouseMove(x, y);
    });
    
    glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow *window, int w, int h) {
        GlfwWindow::FromNativeWindow(window)->OnResize(w, h);
    });
    
}

void GlfwWindow::MakeContextCurrent()
{
    GLFWwindow* actualContext = glfwGetCurrentContext();

    if (actualContext != m_window)
    {
        glfwMakeContextCurrent(m_window);

        //in common case gl functions must be updated after context switching, but fortunately in case "one context per one thread" it don't occured
        TryUpdateGlBindings();
    }
}