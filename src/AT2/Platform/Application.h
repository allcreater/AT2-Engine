#pragma once

#include <AT2/AT2.h>
#include "Common.h"

namespace AT2
{
    //Base class for window-specific user data.
    class GraphicsContext : public IWindow::IWindowContext
    {
    public:
        virtual IWindow& getWindow() override { return *m_window; }
        virtual const IWindow& getWindow() const override { return *m_window; }

        //Renderer and resources should not be exposed outside of the GraphicsContext
        const std::shared_ptr<IRenderer>& getRenderer() const noexcept { return m_renderer; }

        virtual void OnInitialized() {}
        virtual void OnUpdate(Seconds deltaTime) = 0;
        virtual void OnRender(Seconds deltaTime) = 0;
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
        friend class SingleWindowApplication;
        void Initialize(std::shared_ptr<IWindow> window);

    private:
        std::shared_ptr<IWindow> m_window;
        std::shared_ptr<IRenderer> m_renderer;

    };

	class SingleWindowApplication final
    {
    public:        
        void Run(std::unique_ptr<GraphicsContext> graphicsContext);
    };
}