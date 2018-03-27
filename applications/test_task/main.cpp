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


namespace UI
{
	class UiRenderingVisitor : public Visitor
	{
	public:
		UiRenderingVisitor(std::shared_ptr<AT2::IRenderer>& renderer) : m_renderer(renderer)
		{
			auto postprocessShader = renderer->GetResourceFactory().CreateShaderProgramFromFiles(
				{
					"resources//shaders//simple.vs.glsl",
					"resources//shaders//simple.fs.glsl"
				});


			m_quadDrawable = AT2::MeshDrawable::MakeFullscreenQuadDrawable(renderer);
			m_quadDrawable->Shader = postprocessShader;
			//m_quadDrawable->Textures = { Stage2FBO->GetColorAttachement(0), Stage2FBO->GetDepthAttachement(), Noise3Tex, Stage1FBO->GetColorAttachement(0), GrassTex };
			{
				auto uniformStorage = std::make_shared<AT2::GlUniformContainer>(std::dynamic_pointer_cast<AT2::GlShaderProgram>(postprocessShader));
				//uniformStorage->SetUniform("u_phase", Phase);
				//uniformStorage->SetUniform("u_texNoise", Noise3Tex);
				//uniformStorage->SetUniform("u_colorMap", Stage2FBO->GetColorAttachement(0));
				//uniformStorage->SetUniform("u_depthMap", Stage2FBO->GetDepthAttachement());
				m_quadDrawable->UniformBuffer = uniformStorage;
			}
		}

		virtual void Visit(Node& ref) override
		{
			glViewport(ref.GetCanvasData().Position.x, ref.GetCanvasData().Position.y, ref.GetCanvasData().MeasuredSize.x, ref.GetCanvasData().MeasuredSize.y);
			Render(ref);
		}
		virtual void Visit(Group& ref)
		{
			//Render();
		}
		virtual void Visit(StackPanel& ref)
		{
//			Render();
			TraverseChildren(ref);
		}

		void TraverseChildren(Group& group)
		{
			group.ForEachChild([this](std::shared_ptr<Node>& nodePtr) {nodePtr->Accept(*this); });
		}

		void Render(Node& ref)
		{
			std::hash<std::string> hash_fn;
			auto h = hash_fn(std::string(ref.GetName()));
			m_quadDrawable->UniformBuffer->SetUniform("u_Color", glm::vec4((h % 317) / 317.0, (h % 413) / 413.0, (h % 511) / 511.0, 1.0));
			m_quadDrawable->Draw(m_renderer.lock());
		}

	private:
		std::shared_ptr<AT2::MeshDrawable> m_quadDrawable;
		std::weak_ptr<AT2::IRenderer> m_renderer;
	};
}

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

		m_uiRenderer = std::make_unique<UiRenderingVisitor>(m_renderer);
	}

	void OnInitialize()
	{
		m_renderer = std::make_unique<AT2::GlRenderer>();

		CreateUI();
		


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

		m_uiRoot->Accept(*m_uiRenderer.get());

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
	std::shared_ptr<AT2::IRenderer> m_renderer;
	std::shared_ptr<UI::Node> m_uiRoot;

	std::unique_ptr<UI::UiRenderingVisitor> m_uiRenderer;
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