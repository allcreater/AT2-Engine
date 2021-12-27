#pragma once

#include <atomic>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <vector>

#include "window.h"

namespace AT2::SDL
{
    class Exception : public std::runtime_error
    {
    public:
        Exception(const char* reason) : std::runtime_error(reason) {}
    };


    class ConcreteApplication
    {
    public:
        static ConcreteApplication& get();

        ConcreteApplication(const ConcreteApplication&) = delete;
        ConcreteApplication(ConcreteApplication&&) = delete;
        ConcreteApplication& operator=(const ConcreteApplication&) = delete;
        ConcreteApplication& operator=(ConcreteApplication&&) = delete;

        std::function<void()> OnNoActiveWindows {};

        //thread-safe
        std::shared_ptr<Window> createWindow(ContextParameters, glm::ivec2 size);
        std::shared_ptr<Window> createFullscreenWindow(ContextParameters);

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
        ConcreteApplication();
        ~ConcreteApplication();

        std::shared_ptr<Window> createWindowInternal(ContextParameters, glm::ivec2 size);

    private:
        std::atomic<bool> runned {false};
        std::mutex windows_registry_mutex, tasks_mutex;
        std::vector<std::shared_ptr<Window>> windows_registry;

        std::queue<std::packaged_task<void()>> tasks;
    };

} // namespace AT2::SDL