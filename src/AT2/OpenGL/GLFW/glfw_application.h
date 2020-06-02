#pragma once

#include <memory>
#include <mutex>
#include <atomic>
#include <vector>
#include <functional>
#include <future>
#include <queue>



#include "glfw_window.h"

class GlfwException : public std::runtime_error
{
public:
    GlfwException(const char* reason) : std::runtime_error(reason)
    {
    }
};


class GlfwApplication
{
public:
    static GlfwApplication& get();

    GlfwApplication(const GlfwApplication&) = delete;
    GlfwApplication(GlfwApplication&&) = delete;
    GlfwApplication& operator=(const GlfwApplication&) = delete;
    GlfwApplication& operator=(GlfwApplication&&) = delete;

    std::function<void()> OnNoActiveWindows{};

    //thread-safe
    std::shared_ptr<GlfwWindow> createWindow(GlfwContextParameters = {});

    //thread-safe
    template <typename T>
    std::future<void> postAction(T&& func)
    {
        std::lock_guard lock{ tasks_mutex };
        tasks.emplace(std::forward<T>(func));
        
        return tasks.back().get_future();
    }

    void run();
    void stop();

private:
    GlfwApplication();
    ~GlfwApplication();

private:
    std::atomic<bool> runned { false };
    std::mutex windows_registry_mutex, tasks_mutex;
    std::vector<std::shared_ptr<GlfwWindow>> windows_registry;

    std::queue<std::packaged_task<void()>> tasks;
};
