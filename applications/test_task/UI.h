#ifndef RENDERABLE_UI_HEADER
#define RENDERABLE_UI_HEADER

#include <UI/InputHelper.h>

#include <utility>
#include "LinesHelper.h"
#include <Mesh.h>

namespace AT2::UI
{
    class CurveDrawable;

    //encapsulates all visualisation logics of Plot control
    class PlotRenderer : public virtual IUiRenderer, protected LinesHelper
    {
    public:
        PlotRenderer(std::weak_ptr<Plot> plot) : m_Control(std::move(plot)) {}


        void Draw(IRenderer& renderer) override;

    protected:
        void PrepareData(IRenderer& renderer);
        void Init(const IVisualizationSystem& renderer);
        void UpdateCanvasGeometry(const AABB2d& observingRange);

    private:
        std::weak_ptr<Plot> m_Control;

        std::shared_ptr<IPipelineState> m_uiPipeline, m_curvePipeline;
        std::shared_ptr<IUniformContainer> m_uniformBuffer;

        std::map<std::string, std::shared_ptr<CurveDrawable>> m_curves;
        glm::mat4 m_projectionMatrix {1.0};
    };

    struct WindowRenderer : public IUiRenderer
    {
        WindowRenderer(std::weak_ptr<Node> node, MeshRef mesh,
                       const glm::vec2& borderThickness = glm::vec2(3.0, 3.0),
                       const glm::vec4& color = glm::vec4(0.5, 0.5, 0.5, 0.3)) :
            m_Control(std::move(node)),
            m_Mesh(std::move(mesh)), m_borderThickness(borderThickness), m_Color(color)
        {
        }

        void Draw(IRenderer& renderer) override;

    private:
        std::weak_ptr<Node> m_Control;
        MeshRef m_Mesh;

        //visual parameters
        glm::vec2 m_borderThickness, m_blurDirection = glm::vec2(0.7, 0.7);
        glm::vec4 m_Color;
    };


} // namespace AT2::UI
#endif
