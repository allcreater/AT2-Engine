#include "application.h"

//TODO: burn with fire!
namespace AT2::Keys
{
    extern const int Key_W = SDL_SCANCODE_W;
    extern const int Key_S = SDL_SCANCODE_S;
    extern const int Key_A = SDL_SCANCODE_A;
    extern const int Key_D = SDL_SCANCODE_D;
    extern const int Key_Z = SDL_SCANCODE_Z;
    extern const int Key_L = SDL_SCANCODE_L;
    extern const int Key_R = SDL_SCANCODE_R;
    extern const int Key_M = SDL_SCANCODE_M;
    extern const int Key_LShift = SDL_SCANCODE_LSHIFT;
    extern const int Key_Escape = SDL_SCANCODE_ESCAPE;
    extern const int Key_Equal = SDL_SCANCODE_EQUALS;
    extern const int Key_Minus = SDL_SCANCODE_MINUS;
} // namespace AT2::Keys

using namespace AT2::SDL;

ConcreteApplication& ConcreteApplication::get()
{
    static ConcreteApplication keeper;
    return keeper;
}

std::shared_ptr<Window> ConcreteApplication::createWindow(const ContextParameters& parameters, glm::ivec2 size)
{
    return createWindowInternal(parameters, size);
}

std::shared_ptr<Window> ConcreteApplication::createFullscreenWindow(const ContextParameters& parameters)
{
    auto window = createWindowInternal(parameters, {0, 0});
    SDL_SetWindowFullscreen(window->get(), SDL_WINDOW_FULLSCREEN_DESKTOP);
    return window;
}


/*
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
*/

std::shared_ptr<Window> ConcreteApplication::createWindowInternal(const ContextParameters& params, glm::ivec2 size)
{
    auto* const pWindow = new Window(params, size);
    std::shared_ptr<Window> window {pWindow};

    std::lock_guard lock {windows_registry_mutex};
    windows_registry.push_back(window);

    return window;
}

void ConcreteApplication::run()
{
    if (runned)
        throw Exception {"Already running!"};


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


        if (windows_registry.empty() && OnNoActiveWindows)
            OnNoActiveWindows();

		SDL_Event sdlEvent;
        while (SDL_PollEvent(&sdlEvent))
        {
            switch (sdlEvent.type)
            {
            case SDL_WINDOWEVENT:

                switch (sdlEvent.window.type)
                {
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                case SDL_WINDOWEVENT_RESIZED: 
                    DoOnWindow(sdlEvent.window.windowID, [e = sdlEvent.window](Window& wnd) { wnd.OnResize({e.data1, e.data2}); });
                	break;

	            case SDL_WINDOWEVENT_EXPOSED: 
                    DoOnWindow(sdlEvent.window.windowID, [](Window& wnd) { wnd.OnWindowRefreshing(); });
                	break;

                default: 
                    break;
                }

                break;

            case SDL_KEYDOWN:
                DoOnWindow(sdlEvent.key.windowID, [e = sdlEvent.key](Window& wnd) { wnd.OnKeyDown(e.keysym.scancode); });
                break;

            case SDL_KEYUP:
                DoOnWindow(sdlEvent.key.windowID, [e = sdlEvent.key](Window& wnd) { wnd.OnKeyUp(e.keysym.scancode); });
                break;

            case SDL_MOUSEMOTION:
                if (SDL_GetRelativeMouseMode()) //TODO: bypass crutches
					DoOnWindow(sdlEvent.motion.windowID, [e = sdlEvent.motion](Window& wnd) { wnd.MoveMouse({e.xrel, -e.yrel}); });
                else
					DoOnWindow(sdlEvent.motion.windowID, [e = sdlEvent.motion](Window& wnd) { wnd.OnMouseMove({e.x, e.y}); });

                break;

            case SDL_MOUSEBUTTONDOWN:
                DoOnWindow(sdlEvent.button.windowID, [e = sdlEvent.button](Window& wnd) { wnd.OnMouseDown( e.button - 1 ); });
                break;

            case SDL_MOUSEBUTTONUP:
                DoOnWindow(sdlEvent.button.windowID, [e = sdlEvent.button](Window& wnd) { wnd.OnMouseUp( e.button - 1 ); });
                break;

            case SDL_MOUSEWHEEL:
                DoOnWindow(sdlEvent.wheel.windowID, [e = sdlEvent.wheel](Window& wnd) { wnd.OnMouseScroll({e.preciseX, e.preciseY}); });
                break;

            default:
				break;
            }
        }
    }
}

void ConcreteApplication::stop()
{
    if (!runned.exchange(false))
        throw Exception {"Isn't running!"};
}

ConcreteApplication::ConcreteApplication()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        throw Exception("Initialization failed");
}

ConcreteApplication::~ConcreteApplication()
{
    SDL_Quit();
}
