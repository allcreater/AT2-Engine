#include "UIHandler.h"

#include <AT2/TextureLoader.h>

#include <random>
#include <glm/gtx/vec_swizzle.hpp>
#include <utility>

using namespace AT2;
using namespace AT2::UI;

static std::vector<float> GenerateCurve(size_t numPoints, float amplitude, size_t numHarmonics = 10)
{
	static std::mt19937 randGenerator;
	std::uniform_real_distribution<float> frequencyDistribution(0.0001f, 0.3f);
	std::uniform_real_distribution<float> phaseDistribution(0.0f, float(pi * 2));
	std::uniform_real_distribution<float> amplitudeDistribution(0.0f, 1.0f);

	std::vector<std::tuple<float, float, float>> harmonics(numHarmonics);
	for (size_t i = 0; i < harmonics.size(); ++i)
		harmonics[i] = std::make_tuple(frequencyDistribution(randGenerator), phaseDistribution(randGenerator), amplitudeDistribution(randGenerator));

	std::vector<float> data(numPoints);
	for (size_t i = 0; i < numPoints; ++i)
	{
		data[i] = 0.0f;
		for (size_t j = 0; j < harmonics.size(); ++j)
		{
			const auto&[freq, phase, amplitude] = harmonics[j];
			data[i] += sin(i * freq + phase) * amplitude;
		}
	}


	return data;
}

struct PlotCurveSwitchingAnimation : public IAnimation
{
	PlotCurveSwitchingAnimation(float duration, std::weak_ptr<Plot> plotNode, std::string_view hidingCurveName, std::string_view appearingCurveName) :
		m_Duration(duration),
		m_plotNode(std::move(plotNode)),
		m_hidingCurveName(hidingCurveName),
		m_appearingCurveName(appearingCurveName)
	{
	}

	void Animate(double dt) override
	{
		const auto t = m_elapsedTime / m_Duration;

		if (auto plot = m_plotNode.lock())
		{
			auto& hidingCurve = plot->GetOrCreateCurve(m_hidingCurveName);
			auto& appearingCurve = plot->GetOrCreateCurve(m_appearingCurveName);

			hidingCurve.SetColor(glm::vec4(xyz(hidingCurve.GetColor()), 1.0f - t));
			appearingCurve.SetColor(glm::vec4(xyz(appearingCurve.GetColor()), t));
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
	double m_Duration, m_elapsedTime = 0.0;
	std::string m_hidingCurveName, m_appearingCurveName;
};

UiRenderer::UiRenderer(const std::shared_ptr<IRenderer>& renderer, std::shared_ptr<Node> node):
    m_renderer(renderer),
    m_uiRoot(std::move(node))
{
    auto postprocessShader = renderer->GetResourceFactory().CreateShaderProgramFromFiles(
        {
            R"(resources/shaders/background.vs.glsl)",
            R"(resources/shaders/background.fs.glsl)"
        });

    auto texture = AT2::TextureLoader::LoadTexture(renderer, R"(resources/helix_nebula.jpg)");

    m_quadDrawable = AT2::MeshDrawable::MakeFullscreenQuadDrawable(*renderer);
    m_quadDrawable->Shader = postprocessShader;
    m_quadDrawable->Textures = {texture};
    {
        auto uniformStorage = postprocessShader->CreateAssociatedUniformStorage();
        //uniformStorage->SetUniform("u_phase", Phase);
        uniformStorage->SetUniform("u_BackgroundTexture", texture);
        //uniformStorage->SetUniform("u_colorMap", Stage2FBO->GetColorAttachement(0));
        //uniformStorage->SetUniform("u_depthMap", Stage2FBO->GetDepthAttachement());
        m_quadDrawable->UniformBuffer = uniformStorage;
    }
}

void UiRenderer::Draw()
{
    if (auto renderer = m_renderer.lock())
    {
        renderer->SetViewport(AABB2d({}, m_windowSize));
        m_quadDrawable->UniformBuffer->SetUniform("u_Color", glm::vec4(1.0f));
        m_quadDrawable->Draw(*renderer);

        m_uiRoot->TraverseBreadthFirst( [this](const std::shared_ptr<Node>& node) {
            RenderNode(*node);
        });
    }
}

void UiRenderer::RenderNode(const Node& node)
{
    if (auto renderer = m_renderer.lock())
    {
        auto aabb = node.GetScreenPosition();
        renderer->SetViewport(aabb);

        //m_quadDrawable->UniformBuffer->SetUniform("u_Color", DebugColor(node));
        //m_quadDrawable->Draw(m_renderer.lock());

        if (auto nr = node.GetNodeRenderer().lock())
            nr->Draw(*renderer);
    }
}

glm::vec4 UiRenderer::DebugColor(const Node& node)
{
    std::hash<std::string> hash_fn;
    auto h = hash_fn(std::string(node.GetName()));
    return glm::vec4((h % 317) / 317.0, (h % 413) / 413.0, (h % 511) / 511.0, 1.0);
}

void UiHub::Init(std::shared_ptr<AT2::IRenderer>& renderer)
{
	std::shared_ptr<Node> panel, button1, button2;

	m_uiRoot = Group::Make("MainGroup", {
		m_plotNode = Plot::Make("Plot"),
			panel = StackPanel::Make("SidePanel", Orientation::Vertical,
				{
					button1 = Button::Make("ButtonDatasetOne", glm::ivec2(200, 0), Alignment::Stretch, Alignment::Side2),
					button2 = Button::Make("ButtonDatasetTwo", glm::ivec2(200, 0), Alignment::Stretch, Alignment::Side2)
				}, glm::ivec2(210, 0), Alignment::Stretch, Alignment::Side2)
		});

	{
		auto &curve = m_plotNode->GetOrCreateCurve(DataSet1);
		curve.SetData(GenerateCurve(10000, 5.0, 10));
		curve.SetColor(glm::vec4(1.0, 0.0, 0.0, 1.0));
	}
	{
		auto& curve = m_plotNode->GetOrCreateCurve(DataSet2);
		curve.SetData(GenerateCurve(20000, 3.0, 40));
		curve.SetColor(glm::vec4(0.0, 0.0, 1.0, 1.0));
	}


	auto windowRendererSharedData = std::make_shared<WindowRendererSharedInfo>(*renderer);
	m_plotNode->SetNodeRenderer(std::make_shared<PlotRenderer>(m_plotNode));
	panel->SetNodeRenderer(std::make_shared<WindowRenderer>(panel, windowRendererSharedData, glm::vec2(0, 0), glm::vec4(0.5, 0.5, 0.5, 0.5)));
	button1->SetNodeRenderer(std::make_shared<WindowRenderer>(button1, windowRendererSharedData, glm::vec2(4, 4), glm::vec4(2.0, 0.5, 0.5, 0.6)));
	button2->SetNodeRenderer(std::make_shared<WindowRenderer>(button2, windowRendererSharedData, glm::vec2(4, 4), glm::vec4(0.5, 0.5, 2.0, 0.6)));

	auto bounds = m_plotNode->GetAABB();
	m_plotNode->SetObservingZone(AABB2d(glm::vec2(0.0, bounds.MinBound.y), glm::vec2(1000.0, bounds.MaxBound.y)));

	m_uiRenderer = std::make_unique<UiRenderer>(renderer, m_uiRoot);
	m_uiInputHandler = std::make_unique<UiInputHandler>(m_uiRoot);

	m_uiInputHandler->EventClicked = [&](const std::shared_ptr<Node>& node)
	{
		if (node->GetName() == "ButtonDatasetOne" && m_plotNode->GetOrCreateCurve(DataSet2).GetColor().a >= 0.95f)
		{
			m_animationsList.push_back( std::make_unique<PlotCurveSwitchingAnimation>(1.0f, m_plotNode, DataSet2, DataSet1));
			return true;
		}
		else if (node->GetName() == "ButtonDatasetTwo"  && m_plotNode->GetOrCreateCurve(DataSet1).GetColor().a >= 0.95f)
		{
            m_animationsList.push_back(std::make_unique<PlotCurveSwitchingAnimation>(1.0f, m_plotNode, DataSet1, DataSet2));
			return true;
		}
		return false;
	};

	m_uiInputHandler->EventScrolled = [](const std::shared_ptr<Node>& node, const MousePos& mousePos, const glm::vec2& scrollDir)
	{
		if (auto plot = std::dynamic_pointer_cast<Plot>(node); node->GetName() == "Plot")
		{
			auto plotBounds = plot->GetObservingZone();
			float scale = 1.0f + scrollDir.y*0.1f;

			//TODO: implement general way for UI coordinate system transitions
			//dirty code :(
			auto scrAABB = plot->GetScreenPosition();
			glm::vec2 localMousePos = (static_cast<glm::vec2>(mousePos.getPos()) - scrAABB.MinBound) * plotBounds.GetSize() / scrAABB.GetSize() + plotBounds.MinBound;

			auto desiredAABB = AABB2d((plotBounds.MinBound - localMousePos)*scale + localMousePos, (plotBounds.MaxBound - localMousePos)*scale + localMousePos);
			if (desiredAABB.GetWidth() >= 200.0 && desiredAABB.GetWidth() <= 1000.0) //technical requirement :)
				plot->SetObservingZone(desiredAABB);

			return true;
		}
		return false;
	};

	m_uiInputHandler->EventMouseDrag = [](const std::shared_ptr<Node>& node, const MousePos& mousePos)
	{
		if (auto plot = std::dynamic_pointer_cast<Plot>(node); node->GetName() == "Plot")
		{
			auto plotBounds = plot->GetObservingZone();
			auto scrAABB = plot->GetScreenPosition();
			glm::vec2 localMouseDelta = static_cast<glm::vec2>(mousePos.getDeltaPos()) * plotBounds.GetSize() / scrAABB.GetSize();

			plot->SetObservingZone(AABB2d(plotBounds.MinBound - localMouseDelta, plotBounds.MaxBound - localMouseDelta));


			return true;
		}
		return false;
	};
}

void UiHub::Render(std::shared_ptr<IRenderer>& renderer, double dt)
{
	for (auto& animation : m_animationsList)
		animation->Animate(dt);

	m_animationsList.remove_if([](std::unique_ptr<IAnimation>& animation) {return animation->IsFinished(); });

	m_uiRenderer->SetWindowSize(m_windowSize);
	m_uiRenderer->Draw();
}

void UiHub::Resize(const glm::ivec2& newSize)
{
	m_windowSize = newSize;

	m_uiRoot->ComputeMinimalSize();
	m_uiRoot->Measure(glm::ivec2(), newSize);
}
