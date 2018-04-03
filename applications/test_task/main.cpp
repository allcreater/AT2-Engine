#include <AT2/OpenGl/GlRenderer.h>
#include <AT2/OpenGl/GlShaderProgram.h>
#include <AT2/OpenGl/GlUniformBuffer.h>
#include <AT2/OpenGl/GlTexture.h>
#include <AT2/OpenGl/GlVertexArray.h>
#include <AT2/OpenGl/GlFrameBuffer.h>
#include <AT2/OpenGl/GlUniformContainer.h>
#include <AT2/OpenGl/GlTimerQuery.h>

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
			auto aabb = node->GetScreenPosition();
			glViewport(aabb.MinBound.x, m_windowSize.y - aabb.MinBound.y - aabb.GetHeight(), aabb.GetWidth(), aabb.GetHeight());

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

}

struct IAnimation
{
	virtual void Animate(float dt) = 0;
	virtual bool IsFinished() = 0;

	virtual ~IAnimation() = default;
};

struct PlotCurveSwitchingAnimation : public IAnimation
{
	PlotCurveSwitchingAnimation(float duration, std::weak_ptr<AT2::UI::Plot> plotNode, std::string_view hidingCurveName, std::string_view appearingCurveName) : 
		m_Duration(duration), 
		m_plotNode(plotNode),
		m_hidingCurveName(hidingCurveName),
		m_appearingCurveName(appearingCurveName)
	{
	}

	void Animate(float dt) override
	{
		float t = m_elapsedTime / m_Duration;

		if (auto plot = m_plotNode.lock())
		{
			auto& hidingCurve = plot->GetOrCreateCurve(m_hidingCurveName);
			auto& appearingCurve = plot->GetOrCreateCurve(m_appearingCurveName);

			hidingCurve.SetColor(glm::vec4(hidingCurve.GetColor().rgb, 1.0f - t));
			appearingCurve.SetColor(glm::vec4(appearingCurve.GetColor().rgb, t));
		}

		m_elapsedTime += dt;
	}

	bool IsFinished() override
	{
		return m_elapsedTime >= m_Duration;
	}

	~PlotCurveSwitchingAnimation() = default;

private:
	std::weak_ptr<AT2::UI::Plot> m_plotNode;
	float m_Duration, m_elapsedTime = 0.0f;
	std::string m_hidingCurveName, m_appearingCurveName;
};

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
	std::vector<float> GenerateCurve(size_t numPoints, float amplitude, size_t numHarmonics = 10)
	{
		std::mt19937 randGenerator;
		std::uniform_real_distribution<float> frequencyDistribution(0.0001f, 0.1f);
		std::uniform_real_distribution<float> phaseDistribution(0.0f, pi * 2);
		std::uniform_real_distribution<float> amplitudeDistribution(0.0f, 1.0f);

		std::vector<std::tuple<float, float, float>> harmonics(numHarmonics);
		for (auto i = 0; i < harmonics.size(); ++i)
			harmonics[i] = std::make_tuple(frequencyDistribution(randGenerator), phaseDistribution(randGenerator), amplitudeDistribution(randGenerator));

		std::vector<float> data(numPoints);
		for (size_t i = 0; i < numPoints; ++i)
		{
			data[i] = 0.0f;
			for (size_t j = 0; j < harmonics.size(); ++j)
			{
				const auto& [freq, phase, amplitude] = harmonics[j];
				data[i] += sin(i * freq + phase) * amplitude;
			}
		}


		return data;
	}

	void CreateUI()
	{
		using namespace std;
		using namespace AT2::UI;

		m_uiRoot = StackPanel::Make("MainPanel", Orientation::Horizontal,
			{
				m_plotNode = Plot::Make("Plot"),
				StackPanel::Make("SidePanel", Orientation::Vertical,
					{
						Button::Make("ButtonDatasetOne", glm::ivec2(200, 0)),
						Button::Make("ButtonDatasetTwo", glm::ivec2(200, 0))
					})
			});

		{
			auto &curve = m_plotNode->GetOrCreateCurve(DataSet1);
			curve.Data = GenerateCurve(10000, 5.0);
			curve.SetXRange(-5000, 5000);
			curve.SetColor(glm::vec4(1.0, 0.0, 0.0, 1.0));
			curve.Dirty();
		}
		{
			auto& curve = m_plotNode->GetOrCreateCurve(DataSet2);
			curve.Data = GenerateCurve(20000, 3.0);
			curve.SetXRange(-10000, 10000);
			curve.SetColor(glm::vec4(0.0, 0.0, 1.0, 1.0));
			curve.Dirty();
		}

		m_plotNode->SetNodeRenderer(std::make_shared<PlotRenderer>(m_plotNode));

		auto bounds = m_plotNode->GetAABB();
		m_plotNode->SetObservingZone(AABB2d(glm::vec2(0.0, bounds.MinBound.y), glm::vec2(1000.0, bounds.MaxBound.y)));


		m_uiRoot->ComputeMinimalSize();
		m_uiRoot->Measure(glm::ivec2(), m_window.getWindowSize());

		m_uiRenderer = std::make_unique<UiRenderer>(m_renderer, m_uiRoot);
		m_uiInputHandler = std::make_unique<UiInputHandler>(m_uiRoot);

		m_uiInputHandler->EventClicked = [&](std::shared_ptr<Node>& node) 
		{
			if (node->GetName() == "ButtonDatasetOne" && m_plotNode->GetOrCreateCurve(DataSet2).GetColor().a >= 0.95f)
			{
				m_animationsList.emplace_back(new PlotCurveSwitchingAnimation(1.0f, m_plotNode, DataSet2, DataSet1));
				return true;
			}
			else if (node->GetName() == "ButtonDatasetTwo"  && m_plotNode->GetOrCreateCurve(DataSet1).GetColor().a >= 0.95f)
			{
				m_animationsList.emplace_back(new PlotCurveSwitchingAnimation(1.0f, m_plotNode, DataSet1, DataSet2));
				return true;
			}
			return false;
		};

		m_uiInputHandler->EventScrolled = [](std::shared_ptr<Node>& node, const MousePos& mousePos, const glm::vec2& scrollDir)
		{
			if (auto plot = std::dynamic_pointer_cast<Plot>(node); node->GetName() == "Plot")
			{
				auto plotBounds = plot->GetObservingZone();
				float scale = 1.0f + scrollDir.y*0.1f;

				//TODO: implement general way for UI coordinate system transitions
				//dirty code :(
				auto scrAABB = plot->GetScreenPosition();
				glm::vec2 localMousePos = (mousePos.getPos() - scrAABB.MinBound) * plotBounds.GetSize() / scrAABB.GetSize() + plotBounds.MinBound;

				plot->SetObservingZone(AABB2d((plotBounds.MinBound - localMousePos)*scale + localMousePos, (plotBounds.MaxBound - localMousePos)*scale + localMousePos));
				return true;
			}
			return false;
		};

		m_uiInputHandler->EventMouseDrag = [](std::shared_ptr<Node>& node, const MousePos& mousePos)
		{
			if (auto plot = std::dynamic_pointer_cast<Plot>(node); node->GetName() == "Plot")
			{
				auto plotBounds = plot->GetObservingZone();
				auto scrAABB = plot->GetScreenPosition();
				glm::vec2 localMouseDelta = mousePos.getDeltaPos() * plotBounds.GetSize() / scrAABB.GetSize();

				plot->SetObservingZone(AABB2d(plotBounds.MinBound - localMouseDelta, plotBounds.MaxBound - localMouseDelta));
				

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

		for (auto& animation : m_animationsList)
			animation->Animate(dt);
		m_animationsList.remove_if([](std::unique_ptr<IAnimation>& animation) {return animation->IsFinished(); });

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
	const std::string DataSet1 = "DataSet #1";
	const std::string DataSet2 = "DataSet #2";

private:
	GlfwWindow m_window;
	std::shared_ptr<AT2::IRenderer> m_renderer;

	std::shared_ptr<AT2::UI::Node> m_uiRoot;
	std::shared_ptr<AT2::UI::Plot> m_plotNode;

	std::unique_ptr<AT2::UI::UiRenderer> m_uiRenderer;
	std::unique_ptr<AT2::UI::UiInputHandler> m_uiInputHandler;

	std::list<std::unique_ptr<IAnimation>> m_animationsList;
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