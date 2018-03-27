#include <AT2/OpenGl/GlRenderer.h>
#include <AT2/OpenGl/GlShaderProgram.h>
#include <AT2/OpenGl/GlUniformBuffer.h>
#include <AT2/OpenGl/GlTexture.h>
#include <AT2/OpenGl/GlVertexArray.h>
#include <AT2/OpenGl/GlFrameBuffer.h>
#include <AT2/OpenGl/GlUniformContainer.h>
#include <AT2/OpenGl/GlTimerQuery.h>
#include <AT2/OpenGL/GLFW/glfw_window.h>

#include "../drawable.h"

#include "UI.h"

class App
{
public:
	App()
	{
		m_window.setWindowLabel("IQ Option Test Task");
		m_window.setWindowSize(1280, 800);

		SetupWindowCallbacks();
	}

	void Run()
	{
		m_window.Run();
	}

private:
	void CreateUI()
	{
		using namespace std;
		using namespace UI;


		m_uiRoot = StackPanel::Make("MainPanel", Alignment::Horizontal,
			{
				Plot::Make("Plot"),
				StackPanel::Make("SidePanel", Alignment::Vertical,
					{
						Button::Make("ButtonDatasetOne", glm::ivec2(100, 100)),
						Button::Make("ButtonDatasetTwo", glm::ivec2(100, 100))
					})
			});
	}

	void OnInitialize()
	{
		m_renderer = std::make_unique<AT2::GlRenderer>();

		CreateUI();
		

		auto postprocessShader = m_renderer->GetResourceFactory().CreateShaderProgramFromFiles(
			{
			"resources//shaders//simple.vs.glsl",
			"resources//shaders//simple.fs.glsl" 
			});


		//Init
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_CULL_FACE);

	}

	void OnRender(double time, double dt)
	{
		glViewport(0, 0, m_window.getWindowSize().x, m_window.getWindowSize().y);
		
	
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
			auto ms = m_uiRoot->ComputeMinimalSize();
			m_uiRoot->Measure(glm::ivec2(0,0), newSize);
		};

		m_window.MouseUpCallback = [](int key)
		{
			std::cout << "Mouse " << key << std::endl;
		};

		m_window.MouseMoveCallback = [&](const MousePos& pos)
		{
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
	std::unique_ptr<AT2::GlRenderer> m_renderer;
	std::shared_ptr<UI::Node> m_uiRoot;
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
	}

	return 0;
}