#include "glfw_application.h"

//#include <GLFW/glfw3.h>


GlfwApplication& GlfwApplication::get()
{
    static GlfwApplication keeper;
    return keeper;
}


std::shared_ptr<GlfwWindow> GlfwApplication::createWindow(GlfwContextParameters parameters)
{
    auto pWindow = new GlfwWindow(parameters);
    std::shared_ptr<GlfwWindow> window { pWindow };

    std::lock_guard lock { windows_registry_mutex };
    windows_registry.push_back(window);

    return window;
}

void GlfwApplication::run()
{
    if (runned)
        throw GlfwException { "Already running!" };

    runned = true;

    while (runned)
    {
        //Process all delayed actions at main thread
        {
            std::lock_guard lock{ tasks_mutex };
            while (!tasks.empty())
            {
                tasks.front()();
                tasks.pop();
            }
        }

        {
            std::lock_guard lock{ windows_registry_mutex };

            windows_registry.erase(std::remove_if(std::begin(windows_registry), std::end(windows_registry), 
           [](std::shared_ptr<GlfwWindow>& window)
                {
                    if (window->getCloseFlag() || window.use_count() == 1)
                    {
                        window->Close();
                        return true; //
                    }

                    window->UpdateAndRender();
                    return false;
                }
            ), std::end(windows_registry));

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
        throw GlfwException{ "Isn't running!" };
}

GlfwApplication::GlfwApplication()
{
    if (!glfwInit())
        throw GlfwException("Initialization failed");

    glfwSetErrorCallback([](int, const char* message)
    {
        throw GlfwException(message);
    });
}

GlfwApplication::~GlfwApplication()
{
    glfwTerminate();
}
