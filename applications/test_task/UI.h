#ifndef RENDERABLE_UI_HEADER
#define RENDERABLE_UI_HEADER
//#include <AT2/UI/UI.h>

#include <string_view>

#include <AT2/OpenGL/GLFW/glfw_window.h>
#include <AT2/UI/InputVisitor.h>
#include "LinesHelper.h"

namespace AT2::UI
{
	class CurveDrawable;
	
	//encapsulates all visualisation logics of Plot control
	class PlotRenderer : public IDrawable, protected LinesHelper
	{
	public:
		PlotRenderer(std::weak_ptr<Plot> plot) : m_Control(plot)
		{
		}


		void Draw(const std::shared_ptr<IRenderer>& renderer) override;

	protected:
		void PrepareData(const std::shared_ptr<IRenderer>& renderer);
		void Init(const std::shared_ptr<IRenderer>& renderer);
		void UpdateCanvasGeometry(const AABB2d& observingRange);

	private:
		std::weak_ptr<Plot> m_Control;

		std::shared_ptr<IShaderProgram> m_uiShader, m_curveShader;
		std::shared_ptr<IUniformContainer> m_uniformBuffer;

		std::map<std::string, std::shared_ptr<CurveDrawable>> m_curves;
		glm::mat4 m_projectionMatrix;
	};


	//determines some UI events and rise callbacks
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

		void OnMouseMove(const MousePos& mousePos);
		void OnMouseDown(int key);
		void OnMouseUp(int key);

		void OnMouseScroll(const glm::vec2& scrollDir);

	protected:
		bool isPointInsideNode(std::shared_ptr<Node>& node, const glm::vec2& pos);

	private:
		std::shared_ptr<Node> m_rootNode;
		MousePos m_mousePos;
		std::map<int, std::vector<std::weak_ptr<Node>>> m_mouseDownOnControl; //yes, it's an overkill, but it's a simplest solution
	};
}
#endif