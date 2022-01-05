#include "glfw_window.h"
#include <cassert>
#include <mutex>

#include "glfw_application.h"
#include "utils.hpp"

using namespace AT2::GLFW;

namespace
{
    constexpr int TranslateCursorMode(CursorMode cursorMode) 
    {
        switch (cursorMode)
        {
        case CursorMode::Normal: return GLFW_CURSOR_NORMAL;
        case CursorMode::Hidden: return GLFW_CURSOR_HIDDEN;
        case CursorMode::Disabled: return GLFW_CURSOR_DISABLED;
        default: 
            throw std::domain_error("CursorMode");
        }
    }

    constexpr int TranslateOpenGlProfile( AT2::OpenglProfile profile)
    {
	    switch ( profile )
	    {
        case AT2::OpenglProfile::Core: return GLFW_OPENGL_CORE_PROFILE;
        case AT2::OpenglProfile::Compat: return GLFW_OPENGL_COMPAT_PROFILE;
        default: 
            throw std::domain_error("OpengProfile");
	    	//return GLFW_OPENGL_ANY_PROFILE;
	    }
    }

    class GlfwUnifiedContext : public AT2::IPlatformGraphicsContext
    {
    public:
        GlfwUnifiedContext(GLFWwindow* window) : window {window} {}

        void setVSyncInterval(int interval) override { glfwSwapInterval(interval); }
        void* getPlatformSwapchain() const override { return nullptr; }
        void makeCurrent() override
        {
            GLFWwindow* actualContext = glfwGetCurrentContext();

            if (actualContext != window)
                glfwMakeContextCurrent(window);
        }
        void swapBuffers() override { glfwSwapBuffers(window); }

    private:
        GLFWwindow* window;
    };
}


Window* Window::FromNativeWindow(const GLFWwindow* window)
{
    auto* const frontendPtr = static_cast<Window*>(glfwGetWindowUserPointer(const_cast<GLFWwindow*>(window)));
    assert(frontendPtr);
    return frontendPtr;
}

Window::Window(ContextParameters contextParams, glm::ivec2 initialSize, GLFWmonitor* monitor)
	: WindowBase( initialSize)
{
    //std::lock_guard lock(ConcreteApplication::Get().mutex);

    //glfwDefaultWindowHints();

    std::visit(Utils::overloaded {
    	[](const OpenGLContextParams& params) {
          glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, params.context_major_version);
          glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, params.context_minor_version);
          glfwWindowHint(GLFW_OPENGL_PROFILE, TranslateOpenGlProfile(params.profile));
          glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
          glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, params.debug_context);
    	},
    	[](const auto&) { glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); }
    }, contextParams.contextType);

    glfwWindowHint(GLFW_RED_BITS, contextParams.framebuffer_bits_red);
    glfwWindowHint(GLFW_GREEN_BITS, contextParams.framebuffer_bits_green);
    glfwWindowHint(GLFW_BLUE_BITS, contextParams.framebuffer_bits_blue);
    glfwWindowHint(GLFW_DEPTH_BITS, contextParams.framebuffer_bits_depth);

    glfwWindowHint(GLFW_SAMPLES, contextParams.msaa_samples);
    glfwWindowHint(GLFW_REFRESH_RATE, contextParams.refresh_rate);
    glfwWindowHint(GLFW_SRGB_CAPABLE, contextParams.srgb_capable);

    /* Create a windowed mode window and its OpenGL context */
    window_impl = glfwCreateWindow(window_size.x, window_size.y, window_label.c_str(), monitor, nullptr);
    if (!window_impl)
        throw GlfwException("Window creation failed");

    glfwSetWindowUserPointer(window_impl, this);

    graphicsContext = std::make_unique<GlfwUnifiedContext>(window_impl);

    SetupCallbacks();
}

bool Window::isKeyDown(int keyCode) const
{
    return window_impl && (glfwGetKey(window_impl, keyCode) == GLFW_PRESS);
}

bool Window::isMouseKeyDown(int button) const
{
    return window_impl && (glfwGetMouseButton(window_impl, button) == GLFW_PRESS);
}

Window& Window::setCursorMode(CursorMode cursorMode)
{
    ConcreteApplication::get().postAction([=] {
        if (window_impl)
            glfwSetInputMode(window_impl, GLFW_CURSOR, TranslateCursorMode(cursorMode));
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
            glfwSetWindowTitle(window_impl, window_label.c_str());
    });

    return *this;
}

Window& Window::setVSyncInterval(int interval)
{
    ConcreteApplication::get().postAction([this, interval] {
        graphicsContext->makeCurrent();
        graphicsContext->setVSyncInterval(interval);
    });

    return *this;
}

Window& Window::setCloseFlag(bool flag)
{
    std::lock_guard lock {mutex};

    if (window_impl)
    {
        glfwSetWindowShouldClose(window_impl, flag ? GLFW_TRUE : GLFW_FALSE);

        //When flag is setted manually there is no callback, so we should call it manually
        if (flag)
            OnClosing();
    }

    return *this;
}

bool Window::getCloseFlag() const
{
    if (window_impl)
        return glfwWindowShouldClose(window_impl);

    return false;
}

void Window::requestAttention()
{
    ConcreteApplication::get().postAction([=] {
        if (window_impl)
            glfwRequestWindowAttention(window_impl);
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
            glfwSetWindowSize(window_impl, window_size.x, window_size.y);
    });

    return *this;
}

void Window::SetupCallbacks()
{

    glfwSetKeyCallback(window_impl, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto* const wnd = Window::FromNativeWindow(window);

        switch (action)
        {
        case GLFW_RELEASE: wnd->OnKeyUp(key); break;
        case GLFW_PRESS: wnd->OnKeyDown(key); break;
        case GLFW_REPEAT: wnd->OnKeyRepeat(key); break;
        default: throw GlfwException("Unknown key action");
        }
    });

    glfwSetMouseButtonCallback(window_impl, [](GLFWwindow* window, int button, int action, int mods) {
        auto* const wnd = FromNativeWindow(window);
        if (action == GLFW_PRESS)
            wnd->OnMouseDown(button);
        else if (action == GLFW_RELEASE)
            wnd->OnMouseUp(button);
    });

    glfwSetCursorPosCallback(window_impl, [](GLFWwindow* window, double x, double y) {
        FromNativeWindow(window)->OnMouseMove({x, y});
    });

    glfwSetFramebufferSizeCallback(window_impl, [](GLFWwindow* window, int w, int h) {
        FromNativeWindow(window)->OnResize({w, h});
    });

    glfwSetWindowCloseCallback(window_impl, [](GLFWwindow* window) { FromNativeWindow(window)->OnClosing(); });

    glfwSetWindowRefreshCallback(window_impl, [](GLFWwindow* window) {
        auto* const wnd = FromNativeWindow(window);
        wnd->OnWindowRefreshing();
        //wnd->UpdateAndRender();
    });

    glfwSetScrollCallback(window_impl, [](GLFWwindow* window, double offsetX, double offsetY) {
        FromNativeWindow(window)->OnMouseScroll({offsetX, offsetY});
    });
}

void Window::Close()
{
    std::lock_guard lock {mutex};

    assert(window_impl);

    graphicsContext = nullptr; // must be deinitialized before window itself
    glfwDestroyWindow(window_impl);

    window_impl = nullptr;
}
