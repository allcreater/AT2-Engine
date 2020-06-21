#ifndef RENDERABLE_UI_HEADER
#define RENDERABLE_UI_HEADER

#include <AT2/UI/InputHelper.h>

#include <utility>
#include "LinesHelper.h"

namespace AT2::UI
{
	class CurveDrawable;
	
	//encapsulates all visualisation logics of Plot control
	class PlotRenderer : public virtual IDrawable, protected LinesHelper
	{
	public:
		PlotRenderer(std::weak_ptr<Plot> plot) : m_Control(std::move(plot))
		{
		}


		void Draw(const IRenderer& renderer) override;

	protected:
		void PrepareData(const IRenderer& renderer);
		void Init(const IRenderer& renderer);
		void UpdateCanvasGeometry(const AABB2d& observingRange);

	private:
		std::weak_ptr<Plot> m_Control;

		std::shared_ptr<IShaderProgram> m_uiShader, m_curveShader;
		std::shared_ptr<IUniformContainer> m_uniformBuffer;

		std::map<std::string, std::shared_ptr<CurveDrawable>> m_curves;
		glm::mat4 m_projectionMatrix {1.0};
	};


	struct WindowRendererSharedInfo
	{
		WindowRendererSharedInfo(const IRenderer& renderer);

		friend struct WindowRenderer;

	private:
		std::shared_ptr<IVertexArray> m_VAO;
		std::shared_ptr<IShaderProgram> m_Shader;
		std::unique_ptr<IDrawPrimitive> m_DrawPrimitive;
	};

	struct WindowRenderer : public IDrawable
	{
		WindowRenderer(std::weak_ptr<Node> node, std::shared_ptr<WindowRendererSharedInfo> sharedInfo,const glm::vec2& borderThickness = glm::vec2(3.0, 3.0), const glm::vec4& color = glm::vec4(0.5, 0.5, 0.5, 0.3)) :
			m_Control(std::move(node)),
			m_SharedInfo(std::move(sharedInfo)),
			m_borderThickness(borderThickness),
			m_Color(color)
		{
		}

		void Draw(const IRenderer& renderer) override;

	private:
		std::weak_ptr<Node> m_Control;
		std::shared_ptr<WindowRendererSharedInfo> m_SharedInfo;
		std::shared_ptr<IUniformContainer> m_uniforms;

		//visual parameters
		glm::vec2 m_borderThickness, m_blurDirection = glm::vec2(0.7, 0.7);
		glm::vec4 m_Color;
		
	};


}
#endif
