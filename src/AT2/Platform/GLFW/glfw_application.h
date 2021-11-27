#pragma once

#include <atomic>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <vector>


#include "glfw_window.h"

namespace AT2::GLFW
{
    class GlfwException : public std::runtime_error
    {
    public:
        GlfwException(const char* reason) : std::runtime_error(reason) {}
    };


    class GlfwApplication
    {
    public:
        static GlfwApplication& get();

        GlfwApplication(const GlfwApplication&) = delete;
        GlfwApplication(GlfwApplication&&) = delete;
        GlfwApplication& operator=(const GlfwApplication&) = delete;
        GlfwApplication& operator=(GlfwApplication&&) = delete;

        std::function<void()> OnNoActiveWindows {};

        //thread-safe
        std::shared_ptr<GlfwWindow> createWindow(GlfwContextParameters, glm::ivec2 size);
        std::shared_ptr<GlfwWindow> createFullscreenWindow(GlfwContextParameters);

        //thread-safe
        template <typename T>
        std::future<void> postAction(T&& func)
        {
            std::lock_guard lock {tasks_mutex};
            tasks.emplace(std::forward<T>(func));

            return tasks.back().get_future();
        }

        void run();
        void stop();

    private:
        GlfwApplication();
        ~GlfwApplication();

        std::shared_ptr<GlfwWindow> createWindowInternal(GlfwContextParameters, glm::ivec2 size, GLFWmonitor* monitor);

    private:
        std::atomic<bool> runned {false};
        std::mutex windows_registry_mutex, tasks_mutex;
        std::vector<std::shared_ptr<GlfwWindow>> windows_registry;

        std::queue<std::packaged_task<void()>> tasks;
    };

} // namespace AT2::GLFW