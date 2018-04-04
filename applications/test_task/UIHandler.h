#ifndef UI_HANDLER_HEADER
#define UI_HANDLER_HEADER

#include "UI.h"

#include <list>

//TODO: move into library
namespace AT2::UI 
{
	class UiRenderer
	{
	public:
		UiRenderer(std::shared_ptr<AT2::IRenderer>& renderer, std::shared_ptr<AT2::UI::Node> node) : m_renderer(renderer), m_uiRoot(node)
		{
			auto postprocessShader = renderer->GetResourceFactory().CreateShaderProgramFromFiles(
				{
					R"(resources/shaders/background.vs.glsl)",
					R"(resources/shaders/background.fs.glsl)"
				});

			auto texture = renderer->GetResourceFactory().LoadTexture(R"(resources/helix_nebula.jpg)");

			m_quadDrawable = AT2::MeshDrawable::MakeFullscreenQuadDrawable(renderer);
			m_quadDrawable->Shader = postprocessShader;
			m_quadDrawable->Textures = { texture };
			{
				auto uniformStorage = postprocessShader->CreateAssociatedUniformStorage();
				//uniformStorage->SetUniform("u_phase", Phase);
				uniformStorage->SetUniform("u_BackgroundTexture", texture);
				//uniformStorage->SetUniform("u_colorMap", Stage2FBO->GetColorAttachement(0));
				//uniformStorage->SetUniform("u_depthMap", Stage2FBO->GetDepthAttachement());
				m_quadDrawable->UniformBuffer = uniformStorage;
			}
		}

		void Draw()
		{
			glViewport(0, 0, m_windowSize.x, m_windowSize.y);
			m_quadDrawable->UniformBuffer->SetUniform("u_Color", glm::vec4(1.0f));
			m_quadDrawable->Draw(m_renderer.lock());

			m_uiRoot->TraverseBreadthFirst(std::bind(&UiRenderer::RenderNode, this, std::placeholders::_1));
		}

		void SetWindowSize(const glm::uvec2& windowSize) { m_windowSize = windowSize; }

	private:

		void RenderNode(const std::shared_ptr<Node>& node)
		{
			auto aabb = node->GetScreenPosition();
			glViewport(aabb.MinBound.x, m_windowSize.y - aabb.MinBound.y - aabb.GetHeight(), aabb.GetWidth(), aabb.GetHeight());

			//m_quadDrawable->UniformBuffer->SetUniform("u_Color", DebugColor(node));
			//m_quadDrawable->Draw(m_renderer.lock());

			if (auto nr = node->GetNodeRenderer().lock())
				nr->Draw(m_renderer.lock());
		}

		glm::vec4 DebugColor(const std::shared_ptr<Node>& node)
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
	virtual void Animate(double dt) = 0;
	virtual bool IsFinished() = 0;

	virtual ~IAnimation() = default;
};

struct UiHub
{
	UiHub() {}

	AT2::UI::UiInputHandler& GetInputHandler() { return *m_uiInputHandler; }

	void Init(std::shared_ptr<AT2::IRenderer>& renderer);
	void Render(std::shared_ptr<AT2::IRenderer>& renderer, double dt);
	void Resize(const glm::ivec2& newSize);

private:
	const std::string DataSet1 = "DataSet #1";
	const std::string DataSet2 = "DataSet #2";

private:
	glm::ivec2 m_windowSize;

	std::shared_ptr<AT2::UI::Node> m_uiRoot;
	std::shared_ptr<AT2::UI::Plot> m_plotNode;

	std::unique_ptr<AT2::UI::UiRenderer> m_uiRenderer;
	std::unique_ptr<AT2::UI::UiInputHandler> m_uiInputHandler;

	std::list<std::unique_ptr<IAnimation>> m_animationsList;
};

#endif
