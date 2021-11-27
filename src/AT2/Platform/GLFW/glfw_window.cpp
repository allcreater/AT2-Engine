#include "glfw_window.h"
#include <cassert>
#include <mutex>

#include "glfw_application.h"

using namespace AT2::GLFW;

GlfwWindow* GlfwWindow::FromNativeWindow(const GLFWwindow* window)
{
    auto* const frontendPtr = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(const_cast<GLFWwindow*>(window)));
    assert(frontendPtr);
    return frontendPtr;
}

GlfwWindow::GlfwWindow(GlfwContextParameters contextParams, glm::ivec2 initialSize, GLFWmonitor* monitor) :
    context_parameters(contextParams),
    window_size(initialSize)
{
    //std::lock_guard lock(GlfwApplication::Get().mutex);

    //glfwDefaultWindowHints();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, context_parameters.context_major_version);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, context_parameters.context_minor_version);
    glfwWindowHint(GLFW_OPENGL_PROFILE, static_cast<int>(context_parameters.profile));

    glfwWindowHint(GLFW_RED_BITS, context_parameters.framebuffer_bits_red);
    glfwWindowHint(GLFW_GREEN_BITS, context_parameters.framebuffer_bits_green);
    glfwWindowHint(GLFW_BLUE_BITS, context_parameters.framebuffer_bits_blue);

    glfwWindowHint(GLFW_SAMPLES, context_parameters.msaa_samples);
    glfwWindowHint(GLFW_REFRESH_RATE, context_parameters.refresh_rate);
    glfwWindowHint(GLFW_SRGB_CAPABLE, context_parameters.srgb_capable);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, context_parameters.debug_context);

    /* Create a windowed mode window and its OpenGL context */
    window_impl = glfwCreateWindow(window_size.x, window_size.y, window_label.c_str(), monitor, nullptr);
    if (!window_impl)
        throw GlfwException("Window creation failed");

    glfwSetWindowUserPointer(window_impl, this);

    SetupCallbacks();
}

bool GlfwWindow::isKeyDown(int keyCode) const
{
    return window_impl && (glfwGetKey(window_impl, keyCode) == GLFW_PRESS);
}

bool GlfwWindow::isMouseKeyDown(int button) const
{
    return window_impl && (glfwGetMouseButton(window_impl, button) == GLFW_PRESS);
}

GlfwWindow& GlfwWindow::setCursorMode(GlfwCursorMode cursorMode)
{
    GlfwApplication::get().postAction([=] {
        if (window_impl)
            glfwSetInputMode(window_impl, GLFW_CURSOR, static_cast<int>(cursorMode));
    });

    return *this;
}

void GlfwWindow::UpdateAndRender()
{
    assert(window_impl);
    MakeContextCurrent();

    {
        if (!is_initialized)
        {
            is_initialized = true;
            OnInitialize();
            OnResize(getSize());
        }

        const auto currentTime = Seconds { glfwGetTime() };
        OnUpdate(currentTime - previous_render_time);
        if (getSize().x > 0 && getSize().y > 0)
            OnRender(currentTime - previous_render_time);

        previous_render_time = currentTime;
    }

    glfwSwapBuffers(window_impl);
}

GlfwWindow& GlfwWindow::setLabel( std::string label)
{
    {
        std::lock_guard lock {mutex};
        window_label = std::move(label);
    }

    GlfwApplication::get().postAction([=] {
        if (window_impl != nullptr)
            glfwSetWindowTitle(window_impl, label.c_str());
    });

    return *this;
}

GlfwWindow& GlfwWindow::setVSyncInterval(int interval)
{
    std::lock_guard lock {mutex};

    swap_interval = interval;
    swap_interval_need_update = true;

    return *this;
}

GlfwWindow& GlfwWindow::setCloseFlag(bool flag)
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

bool GlfwWindow::getCloseFlag() const
{
    if (window_impl)
        return glfwWindowShouldClose(window_impl);

    return false;
}

void GlfwWindow::requestAttention()
{
    GlfwApplication::get().postAction([=] {
        if (window_impl)
            glfwRequestWindowAttention(window_impl);
    });
}

GlfwWindow& GlfwWindow::setSize(glm::ivec2 size)
{
    {
        std::lock_guard lock {mutex};
        window_size = size;
    }

    GlfwApplication::get().postAction([=] {
        if (window_impl != nullptr)
            glfwSetWindowSize(window_impl, window_size.x, window_size.y);
    });

    return *this;
}

void GlfwWindow::SetupCallbacks()
{

    glfwSetKeyCallback(window_impl, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto* const wnd = GlfwWindow::FromNativeWindow(window);

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

void GlfwWindow::MakeContextCurrent()
{
    GLFWwindow* actualContext = glfwGetCurrentContext();

    if (actualContext != window_impl)
    {
        glfwMakeContextCurrent(window_impl);

        if (swap_interval_need_update)
        {
            glfwSwapInterval(swap_interval);
            swap_interval_need_update = false;
        }
    }
}

void GlfwWindow::Close()
{
    std::lock_guard lock {mutex};

    assert(window_impl);

    glfwDestroyWindow(window_impl);
    window_impl = nullptr;
}
