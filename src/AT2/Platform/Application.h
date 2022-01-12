#pragma once

#include <AT2.h>
#include "Common.h"

namespace AT2
{
    //Base class for window-specific user data.
    class WindowContextBase : public IWindow::IWindowContext
    {
        friend class SingleWindowApplication;

    public:
    	IWindow& getWindow() override { return *m_window; }
        const IWindow& getWindow() const override { return *m_window; }

        //Renderer and resources should not be exposed outside of the WindowContextBase
        //It triggers resource loaders refactoring, but it's okay

        virtual void OnInitialized(IVisualizationSystem& visualizationSystem) {}
        virtual void OnUpdate(Seconds deltaTime) = 0;
        virtual void OnRender(Seconds deltaTime, IVisualizationSystem& visualizationSystem) = 0;
        virtual void OnWindowRefreshing() {}
        virtual void OnKeyDown(int key) {}
        virtual void OnKeyUp(int key) {}
        virtual void OnKeyRepeat(int key) {}
        virtual void OnResize(glm::ivec2 newSize) {}
        virtual void OnClosing() {}
        virtual void OnMouseMove(const MousePos& mousePosition) {}
        virtual void OnMouseDown(int button) {}
        virtual void OnMouseUp(int button) {}
        virtual void OnMouseScroll(const glm::dvec2& scrollDirection) {}


    private:
        IWindow* m_window = nullptr; // guaranteed to be not null after initialization
        std::unique_ptr<IVisualizationSystem> m_visualizationSystem;
    };

	class SingleWindowApplication final
    {
    public:        
        void Run(std::unique_ptr<WindowContextBase> windowContext);
    };
}