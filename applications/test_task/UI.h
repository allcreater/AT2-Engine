#ifndef RENDERABLE_UI_HEADER
#define RENDERABLE_UI_HEADER
#include <AT2/UI/UI.h>
#include "../drawable.h"

namespace AT2::UI
{

	class PlotRenderer : public IDrawable
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

		std::shared_ptr<IVertexArray> m_uiVAO;
		std::shared_ptr<IShaderProgram> m_uiShader, m_curveShader;
		std::shared_ptr<IUniformContainer> m_uniformBuffer;
		PrimitiveList m_uiPrimitiveList;
	};

}
#endif