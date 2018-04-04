#ifndef RENDERABLE_UI_HEADER
#define RENDERABLE_UI_HEADER

#include <AT2/UI/InputHelper.h>
#include "LinesHelper.h"

namespace AT2::UI
{
	class CurveDrawable;
	
	//encapsulates all visualisation logics of Plot control
	class PlotRenderer : public virtual IDrawable, protected LinesHelper
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


	struct WindowRendererSharedInfo
	{
		WindowRendererSharedInfo(const std::shared_ptr<IRenderer>& renderer);

		friend struct WindowRenderer;

	private:
		std::shared_ptr<IVertexArray> m_VAO;
		std::shared_ptr<IShaderProgram> m_Shader;
		std::unique_ptr<IDrawPrimitive> m_DrawPrimitive;
	};

	struct WindowRenderer : public IDrawable
	{
		WindowRenderer(std::weak_ptr<Node> node, std::shared_ptr<WindowRendererSharedInfo> sharedInfo) : m_Control(node), m_SharedInfo(sharedInfo)
		{
		}

		void Draw(const std::shared_ptr<IRenderer>& renderer) override;

	private:
		std::weak_ptr<Node> m_Control;
		std::shared_ptr<WindowRendererSharedInfo> m_SharedInfo;
		std::shared_ptr<IUniformContainer> m_uniforms;
		
	};


}
#endif
