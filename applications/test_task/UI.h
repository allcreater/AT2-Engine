#ifndef RENDERABLE_UI_HEADER
#define RENDERABLE_UI_HEADER
//#include <AT2/UI/UI.h>
#include <AT2/UI/InputVisitor.h>
#include "LinesHelper.h"

namespace AT2::UI
{
	class CurveDrawable;
	
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

	private:
		std::weak_ptr<Plot> m_Control;

		std::shared_ptr<IShaderProgram> m_uiShader, m_curveShader;
		std::shared_ptr<IUniformContainer> m_uniformBuffer;

		std::map<std::string, std::shared_ptr<CurveDrawable>> m_curves;
		glm::mat4 m_projectionMatrix;
	};

}
#endif