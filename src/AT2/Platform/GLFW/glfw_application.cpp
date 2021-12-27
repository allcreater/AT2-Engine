#include "glfw_application.h"

//#include <GLFW/glfw3.h>

using namespace AT2::GLFW;

ConcreteApplication& ConcreteApplication::get()
{
    static ConcreteApplication keeper;
    return keeper;
}


std::shared_ptr<Window> ConcreteApplication::createWindow(ContextParameters parameters, glm::ivec2 size)
{
    return createWindowInternal(parameters, size, nullptr);
}

std::shared_ptr<Window> ConcreteApplication::createFullscreenWindow(ContextParameters parameters)
{
    auto* monitor = glfwGetPrimaryMonitor();
    const auto* defaultVideomode = glfwGetVideoMode(monitor);

    parameters.refresh_rate = defaultVideomode->refreshRate;
    parameters.framebuffer_bits_red = defaultVideomode->redBits;
    parameters.framebuffer_bits_green = defaultVideomode->greenBits;
    parameters.framebuffer_bits_blue = defaultVideomode->blueBits;

    return createWindowInternal(parameters, {defaultVideomode->width, defaultVideomode->height}, monitor);
}

std::shared_ptr<Window> ConcreteApplication::createWindowInternal(ContextParameters parameters, glm::ivec2 size, GLFWmonitor* monitor)
{
    auto* const pWindow = new Window(parameters, size, monitor);
    std::shared_ptr<Window> window {pWindow};

    std::lock_guard lock {windows_registry_mutex};
    windows_registry.push_back(window);

    return window;
}

void ConcreteApplication::run()
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
                                                  [](const std::shared_ptr<Window>& window) {
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

void ConcreteApplication::stop()
{
    if (!runned.exchange(false))
        throw GlfwException {"Isn't running!"};
}

ConcreteApplication::ConcreteApplication()
{
    if (!glfwInit())
        throw GlfwException("Initialization failed");

    glfwSetErrorCallback([](int, const char* message) { throw GlfwException(message); });
}

ConcreteApplication::~ConcreteApplication()
{
    glfwTerminate();
}
