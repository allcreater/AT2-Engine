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
	class UiRenderer
	{
	public:
		UiRenderer(std::shared_ptr<AT2::IRenderer>& renderer, std::shared_ptr<AT2::UI::Node> node) : m_renderer(renderer), m_uiRoot(node)
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

		void Draw()
		{

			m_uiRoot->TraverseBreadthFirst(std::bind(&UiRenderer::RenderNode, this, std::placeholders::_1));
		}

		void SetWindowSize(const glm::uvec2& windowSize) { m_windowSize = windowSize; }

	private:

		void RenderNode(std::shared_ptr<Node>& node)
		{
			glViewport(node->GetCanvasData().Position.x, m_windowSize.y - node->GetCanvasData().Position.y - node->GetCanvasData().MeasuredSize.y, node->GetCanvasData().MeasuredSize.x, node->GetCanvasData().MeasuredSize.y);

			m_quadDrawable->UniformBuffer->SetUniform("u_Color", DebugColor(node));
			m_quadDrawable->Draw(m_renderer.lock());

			if (auto nr = node->GetNodeRenderer().lock())
				nr->Draw(m_renderer.lock());
		}

		glm::vec4 DebugColor(std::shared_ptr<Node>& node)
		{
			std::hash<std::string> hash_fn;
			auto h = hash_fn(std::string(node->GetName()));
			return glm::vec4((h % 317) / 317.0, (h % 413) / 413.0, (h % 511) / 511.0, 1.0);
		}

	private:
		std::shared_ptr<AT2::MeshDrawable> m_quadDrawable;
		std::weak_ptr<AT2::IRenderer> m_renderer;
		std::shared_ptr<AT2::UI::Node> m_uiRoot;
		glm::uvec2 m_windowSize;
	};

	class UiInputHandler
	{
	public:
		UiInputHandler(std::shared_ptr<AT2::UI::Node> rootNode) : m_rootNode(rootNode)
		{

		}

	public:
		std::function<bool(std::shared_ptr<Node>& node)> EventClicked;
		std::function<bool(std::shared_ptr<Node>& node, const glm::vec2& dragDir)> EventMouseDrag;
		std::function<bool(std::shared_ptr<Node>& node, const glm::vec2& scrollDir)> EventScrolled;

		void OnMouseMove(const MousePos& mousePos)
		{
			m_mousePos = mousePos;

			bool eventCatched = false;

			m_rootNode->TraverseDepthFirst([&](std::shared_ptr<Node>& node) {
				if (!eventCatched && isPointInsideNode(node, m_mousePos.getPos()))
				{
					if (auto& vector = m_mouseDownOnControl[0]; std::find_if(vector.begin(), vector.end(), [&](const std::weak_ptr<Node>& n) {return n.lock() == node; }) != vector.end())
						if (EventClicked)
							eventCatched |= EventMouseDrag(node, mousePos.getDeltaPos());
				}
			});

		}

		void OnMouseDown(int key)
		{
			m_rootNode->TraverseDepthFirst([&](std::shared_ptr<Node>& node) {
				if (isPointInsideNode(node, m_mousePos.getPos()))
					m_mouseDownOnControl[key].push_back(node);
			});

		}
		void OnMouseUp(int key)
		{
			bool eventCatched = false;

			if (key == 0)
			{
				m_rootNode->TraverseDepthFirst([&](std::shared_ptr<Node>& node) {
					if (!eventCatched && isPointInsideNode(node, m_mousePos.getPos()))
					{
						if (auto& vector = m_mouseDownOnControl[key]; std::find_if(vector.begin(), vector.end(), [&](const std::weak_ptr<Node>& n) {return n.lock() == node; }) != vector.end())
							if (EventClicked)
								eventCatched |= EventClicked(node);
					}
				});
			}

			m_mouseDownOnControl[key].clear();
		}

		void OnMouseScroll(const glm::vec2& scrollDir)
		{
			bool eventCatched = false;

			m_rootNode->TraverseDepthFirst([&](std::shared_ptr<Node>& node) {
				if (!eventCatched && isPointInsideNode(node, m_mousePos.getPos()))
				{
					if (EventScrolled)
						eventCatched |= EventScrolled(node, scrollDir);

					eventCatched = true;
				}
			});
		}

		bool isPointInsideNode(std::shared_ptr<Node>& node, const glm::vec2& pos)
		{
			auto aabb = AABB2d(node->GetCanvasData().Position, node->GetCanvasData().Position + glm::ivec2(node->GetCanvasData().MeasuredSize));
			return aabb.IsPointInside(pos);
		}

	private:
		std::shared_ptr<Node> m_rootNode;
		MousePos m_mousePos;
		std::map<int, std::vector<std::weak_ptr<Node>>> m_mouseDownOnControl; //yes, it's an overkill, but it's a simplest solution
	};
}

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
	std::vector<float> GenerateCurve(size_t numPoints, float amplitude)
	{
		std::mt19937 randGenerator;
		std::uniform_real_distribution<float> distribution(0.001f, 0.01f);
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

		m_uiRoot = StackPanel::Make("MainPanel", Orientation::Horizontal,
			{
				plot = Plot::Make("Plot"),
				StackPanel::Make("SidePanel", Orientation::Vertical,
					{
						Button::Make("ButtonDatasetOne", glm::ivec2(200, 0)),
						Button::Make("ButtonDatasetTwo", glm::ivec2(200, 0))
					})
			});

		{
			auto &curve = plot->GetOrCreateCurve("DataSet #1");
			curve.Data = GenerateCurve(10000, 5.0);
			curve.SetXRange(-5000, 5000);
			curve.SetColor(glm::vec4(1.0, 0.0, 0.0, 1.0));
			curve.Dirty();
		}
		{
			auto& curve = plot->GetOrCreateCurve("DataSet #2");
			curve.Data = GenerateCurve(20000, 3.0);
			curve.SetXRange(-10000, 10000);
			curve.SetColor(glm::vec4(0.0, 0.0, 1.0, 1.0));
			curve.Dirty();
		}

		plot->SetNodeRenderer(std::make_shared<PlotRenderer>(plot));

		auto bounds = plot->GetAABB();
		plot->SetObservingZone(bounds);


		m_uiRoot->ComputeMinimalSize();
		m_uiRoot->Measure(glm::ivec2(), m_window.getWindowSize());

		m_uiRenderer = std::make_unique<UiRenderer>(m_renderer, m_uiRoot);
		m_uiInputHandler = std::make_unique<UiInputHandler>(m_uiRoot);

		m_uiInputHandler->EventClicked = [](std::shared_ptr<Node>& node) 
		{
			std::cout << std::string(node->GetName()) << " clicked" << std::endl;
			return true;
		};

		m_uiInputHandler->EventScrolled = [](std::shared_ptr<Node>& node, const glm::vec2& scrollDir)
		{
			if (auto plot = std::dynamic_pointer_cast<Plot>(node); node->GetName() == "Plot")
			{
				auto oldBounds = plot->GetObservingZone();
				float scale = 1.0f + scrollDir.y*0.1f;
				plot->SetObservingZone(AABB2d(oldBounds.MinBound*scale, oldBounds.MaxBound*scale));
				return true;
			}
			return false;
		};

		m_uiInputHandler->EventMouseDrag = [](std::shared_ptr<Node>& node, const glm::vec2& delta)
		{
			if (auto plot = std::dynamic_pointer_cast<Plot>(node); node->GetName() == "Plot")
			{
				std::cout << std::string(node->GetName()) << " dragged" << std::endl;
				return true;
			}
			return false;
		};
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
		m_uiRenderer->Draw();

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

		m_window.MouseDownCallback = [&](int key)
		{
			m_uiInputHandler->OnMouseDown(key);
		};

		m_window.MouseUpCallback = [&](int key)
		{
			std::cout << "Mouse " << key << std::endl;
			m_uiInputHandler->OnMouseUp(key);
		};

		m_window.MouseMoveCallback = [&](const MousePos& pos)
		{
			m_uiInputHandler->OnMouseMove(pos);
		};

		m_window.MouseScrollCallback = [&](const glm::vec2& scrollDir)
		{
			std::cout << "Scroll " << scrollDir.y << std::endl;
			m_uiInputHandler->OnMouseScroll(scrollDir);
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
	//std::weak_ptr<AT2::UI::Plot> m_nodePlot;

	std::unique_ptr<AT2::UI::UiRenderer> m_uiRenderer;
	std::unique_ptr<AT2::UI::UiInputHandler> m_uiInputHandler;
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