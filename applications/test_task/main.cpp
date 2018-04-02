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

#include <random>

namespace AT2::UI
{
	class UiRenderingVisitor : public UiVisitor
	{
	public:
		UiRenderingVisitor(std::shared_ptr<AT2::IRenderer>& renderer) : m_renderer(renderer)
		{
			auto postprocessShader = renderer->GetResourceFactory().CreateShaderProgramFromFiles(
				{
					"resources//shaders//window.vs.glsl",
					"resources//shaders//window.fs.glsl"
				});


			m_quadDrawable = AT2::MeshDrawable::MakeFullscreenQuadDrawable(renderer);
			m_quadDrawable->Shader = postprocessShader;
			//m_quadDrawable->Textures = { Stage2FBO->GetColorAttachement(0), Stage2FBO->GetDepthAttachement(), Noise3Tex, Stage1FBO->GetColorAttachement(0), GrassTex };
			{
				auto uniformStorage = postprocessShader->CreateAssociatedUniformStorage();
				//uniformStorage->SetUniform("u_phase", Phase);
				//uniformStorage->SetUniform("u_texNoise", Noise3Tex);
				//uniformStorage->SetUniform("u_colorMap", Stage2FBO->GetColorAttachement(0));
				//uniformStorage->SetUniform("u_depthMap", Stage2FBO->GetDepthAttachement());
				m_quadDrawable->UniformBuffer = uniformStorage;
			}
		}

		void Visit(Node& node) override
		{
			UiVisitor::Visit(node);
		}
		void Visit(Group& node) override
		{
			UiVisitor::Visit(node);
		}
		void Visit(StackPanel& node) override
		{
//			Render();
			UiVisitor::Visit(node);
		}
		void Visit(Button& node) override
		{
			Render(node);
			UiVisitor::Visit(node);
		}
		void Visit(Plot& node) override
		{
			//
			Render(node);

			//glViewport(node.GetCanvasData().Position.x, node.GetCanvasData().Position.y, node.GetCanvasData().MeasuredSize.x, node.GetCanvasData().MeasuredSize.y);
			if (auto nr = node.GetNodeRenderer().lock())
				nr->Draw(m_renderer.lock());

			UiVisitor::Visit(node);
		}

		void SetWindowSize(const glm::uvec2& windowSize) { m_windowSize = windowSize; }

	private:

		void Render(Node& node)
		{
			glViewport(node.GetCanvasData().Position.x, m_windowSize.y - node.GetCanvasData().Position.y - node.GetCanvasData().MeasuredSize.y, node.GetCanvasData().MeasuredSize.x, node.GetCanvasData().MeasuredSize.y);

			m_quadDrawable->UniformBuffer->SetUniform("u_Color", DebugColor(node));
			m_quadDrawable->Draw(m_renderer.lock());
		}

		glm::vec4 DebugColor(Node& node)
		{
			std::hash<std::string> hash_fn;
			auto h = hash_fn(std::string(node.GetName()));
			return glm::vec4((h % 317) / 317.0, (h % 413) / 413.0, (h % 511) / 511.0, 1.0);
		}

	private:
		std::shared_ptr<AT2::MeshDrawable> m_quadDrawable;
		std::weak_ptr<AT2::IRenderer> m_renderer;
		glm::uvec2 m_windowSize;
	};
}

void Foo()
{

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
	std::vector<float> GenerateCurve(size_t numPoints, float amplitude)
	{
		std::mt19937 randGenerator;
		std::uniform_real_distribution<float> distribution(0.001, 0.01);
		auto rnd = std::bind(distribution, randGenerator);

		std::vector<float> data(numPoints);
		float freq = rnd();
		for (size_t i = 0; i < numPoints; ++i)
		{
			data[i] = sin(i * freq) * amplitude;
		}


		return data;
	}

	void CreateUI()
	{
		using namespace std;
		using namespace AT2::UI;

		std::shared_ptr<Plot> plot;
		std::shared_ptr<Node> button;

		m_uiRoot = StackPanel::Make("MainPanel", Orientation::Horizontal,
			{
				plot = Plot::Make("Plot"),
				StackPanel::Make("SidePanel", Orientation::Vertical,
					{
						button = Button::Make("ButtonDatasetOne", glm::ivec2(200, 0)),
						Button::Make("ButtonDatasetTwo", glm::ivec2(200, 0))
					})
			});

		{
			auto &curve = plot->GetOrCreateCurve("DataSet #1");
			curve.Data = GenerateCurve(10000, 5.0);
			curve.SetXRange(-50, 50.0);
			curve.Dirty();
		}
		{
			auto& curve = plot->GetOrCreateCurve("DataSet #2");
			curve.Data = GenerateCurve(20000, 3.0);
			curve.SetXRange(-100, 200);
			curve.Dirty();
		}

		plot->SetNodeRenderer(std::make_shared<PlotRenderer>(plot));

		auto bounds = plot->GetAABB();
		plot->SetObservingZone(bounds);

		button->EventClicked = [&](const Node& node) {
			std::cout << std::string(node.GetName()) << "clicked" << std::endl;
			return true; 
		};

		m_uiRoot->ComputeMinimalSize();
		m_uiRoot->Measure(glm::ivec2(), m_window.getWindowSize());

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

		m_uiRenderer->SetWindowSize(m_window.getWindowSize());
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
			std::cout << "Size " << newSize.x  << "x" << newSize.y << std::endl;
			auto ms = m_uiRoot->ComputeMinimalSize();
			m_uiRoot->Measure(glm::ivec2(0,0), newSize);
		};

		m_window.MouseUpCallback = [&](int key)
		{
			std::cout << "Mouse " << key << std::endl;
			AT2::UI::MouseClickVisitor mcv(m_mousePos);
			m_uiRoot->Accept(mcv);
		};

		m_window.MouseMoveCallback = [&](const MousePos& pos)
		{
			m_mousePos = pos.getPos();
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
	std::shared_ptr<AT2::UI::Node> m_uiRoot;

	std::unique_ptr<AT2::UI::UiRenderingVisitor> m_uiRenderer;

	glm::vec2 m_mousePos;
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