#include "glfw_application.h"

//#include <GLFW/glfw3.h>

using namespace AT2::GLFW;

GlfwApplication& GlfwApplication::get()
{
    static GlfwApplication keeper;
    return keeper;
}


std::shared_ptr<GlfwWindow> GlfwApplication::createWindow(GlfwContextParameters parameters, glm::ivec2 size)
{
    return createWindowInternal(parameters, size, nullptr);
}

std::shared_ptr<GlfwWindow> GlfwApplication::createFullscreenWindow(GlfwContextParameters parameters)
{
    auto* monitor = glfwGetPrimaryMonitor();
    const auto* defaultVideomode = glfwGetVideoMode(monitor);

    if (parameters)
    {
        parameters->refresh_rate = defaultVideomode->refreshRate;
        parameters->framebuffer_bits_red = defaultVideomode->redBits;
        parameters->framebuffer_bits_green = defaultVideomode->greenBits;
        parameters->framebuffer_bits_blue = defaultVideomode->blueBits;
    }

    return createWindowInternal(parameters, {defaultVideomode->width, defaultVideomode->height}, monitor);
}

std::shared_ptr<GlfwWindow> GlfwApplication::createWindowInternal(GlfwContextParameters parameters, glm::ivec2 size, GLFWmonitor* monitor)
{
    auto* const pWindow = new GlfwWindow(parameters, size, monitor);
    std::shared_ptr<GlfwWindow> window {pWindow};

    std::lock_guard lock {windows_registry_mutex};
    windows_registry.push_back(window);

    return window;
}

void GlfwApplication::run()
{
    if (runned)
        throw GlfwException {"Already running!"};

    runned = true;

    while (runned)
    {
        //Process all delayed actions at main thread
        {
            std::lock_guard lock {tasks_mutex};
            while (!tasks.empty())
            {
                tasks.front()();
                tasks.pop();
            }
        }

        {
            std::lock_guard lock {windows_registry_mutex};

            windows_registry.erase(std::remove_if(std::begin(windows_registry), std::end(windows_registry),
                                                  [](const std::shared_ptr<GlfwWindow>& window) {
                                                      if (window->getCloseFlag() || window.use_count() == 1)
                                                      {
                                                          window->Close();
                                                          return true; //
                                                      }

                                                      window->UpdateAndRender();
                                                      return false;
                                                  }),
                                   std::end(windows_registry));
        }

        glfwMakeContextCurrent(nullptr);

        if (windows_registry.empty() && OnNoActiveWindows)
            OnNoActiveWindows();

        glfwPollEvents();
    }
}

void GlfwApplication::stop()
{
    if (!runned.exchange(false))
        throw GlfwException {"Isn't running!"};
}

GlfwApplication::GlfwApplication()
{
    if (!glfwInit())
        throw GlfwException("Initialization failed");

    glfwSetErrorCallback([](int, const char* message) { throw GlfwException(message); });
}

GlfwApplication::~GlfwApplication()
{
    glfwTerminate();
}
