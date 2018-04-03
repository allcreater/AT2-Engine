#include <AT2/OpenGl/GlRenderer.h>
#include <AT2/OpenGl/GlShaderProgram.h>
#include <AT2/OpenGl/GlUniformBuffer.h>
#include <AT2/OpenGl/GlTexture.h>
#include <AT2/OpenGl/GlVertexArray.h>
#include <AT2/OpenGl/GlFrameBuffer.h>
#include <AT2/OpenGl/GlUniformContainer.h>
#include <AT2/OpenGl/GlTimerQuery.h>

#include "../drawable.h"

#include "UIHandler.h"


class App
{
public:
	App()
	{
		m_window.setWindowLabel("Graph control demo");
		m_window.setWindowSize(1280, 800);

		SetupWindowCallbacks();
	}

	void Run()
	{
		m_window.Run();
	}

private:

	void OnInitialize()
	{
		m_renderer = std::make_unique<AT2::GlRenderer>();

		m_uiHub = std::make_unique<UiHub>();
		m_uiHub->Init(m_renderer);
		m_uiHub->Resize(m_window.getWindowSize());

		//Init
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

	}

	void OnRender(double time, double dt)
	{
		glViewport(0, 0, m_window.getWindowSize().x, m_window.getWindowSize().y);
		m_renderer->ClearBuffer(glm::vec4(0.0, 0.0, 0.0, 0.0));
		m_renderer->ClearDepth(0);

		m_uiHub->Render(m_renderer, dt);

		m_renderer->FinishFrame();
	}

	void OnKeyPress(int key)
	{
	}

	void SetupWindowCallbacks()
	{


		m_window.KeyDownCallback = [&](int key)
		{
			std::cout << "Key " << key << " down" << std::endl;

			switch (key)
			{
			case GLFW_KEY_R:
			{
				m_renderer->GetResourceFactory().ReloadResources(AT2::ReloadableGroup::Shaders);
			} break;

			case GLFW_KEY_ESCAPE:
			{
				m_window.setWindowCloseFlag(true);
			} break;

			}


			OnKeyPress(key);
		};

		m_window.KeyRepeatCallback = [&](int key)
		{
			OnKeyPress(key);
		};

		m_window.ResizeCallback = [&](const glm::ivec2& newSize)
		{
			std::cout << "Size " << newSize.x << "x" << newSize.y << std::endl;
			m_uiHub->Resize(newSize);
		};

		m_window.MouseDownCallback = [&](int key)
		{
			m_uiHub->GetInputHandler().OnMouseDown(key);
		};

		m_window.MouseUpCallback = [&](int key)
		{
			std::cout << "Mouse " << key << std::endl;
			m_uiHub->GetInputHandler().OnMouseUp(key);
		};

		m_window.MouseMoveCallback = [&](const MousePos& pos)
		{
			m_uiHub->GetInputHandler().OnMouseMove(pos);
		};

		m_window.MouseScrollCallback = [&](const glm::vec2& scrollDir)
		{
			std::cout << "Scroll " << scrollDir.y << std::endl;
			m_uiHub->GetInputHandler().OnMouseScroll(scrollDir);
		};

		m_window.InitializeCallback = [&]()
		{
			m_window.setVSyncInterval(1);
		};

		m_window.ClosingCallback = [&]()
		{
			m_renderer->Shutdown();
		};

		m_window.RenderCallback = std::bind(&App::OnRender, this, std::placeholders::_1, std::placeholders::_2);
		m_window.InitializeCallback = std::bind(&App::OnInitialize, this);
	}

private:
	GlfwWindow m_window;
	std::shared_ptr<AT2::IRenderer> m_renderer;

	std::unique_ptr<UiHub> m_uiHub;
};

int main(int argc, char *argv[])
{
	try
	{
		InitGLFW();

		App app;
		app.Run();

		ReleaseGLFW();
	}
	catch (AT2::AT2Exception exception)
	{
		std::cout << "Runtime exception:" << exception.what() << std::endl;
		system("PAUSE");
	}

	return 0;
}