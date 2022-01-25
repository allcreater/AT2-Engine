#include "UI.h"

#include "../mesh_renderer.h"

using namespace AT2;
using namespace AT2::UI;

#include <algorithm>

//it is possible to contain all data in one vertex buffer array and one vertex buffer, but it's more complex task and much unclear code, so let's just draw it separately
class AT2::UI::CurveDrawable : public IUiRenderer
{
public:
    void Draw(IRenderer& renderer) override
    {
        if (m_VAO == nullptr)
            Init(renderer);

        auto& stateManager = renderer.GetStateManager();
        stateManager.BindVertexArray(m_VAO);
        stateManager.ApplyState(LineRasterizationMode::Smooth);
        m_uniforms->SetUniform("u_matProjection", m_projectionMatrix);

        m_uniforms->Bind(stateManager);
        renderer.Draw(Primitives::LineStrip {}, 0, static_cast<long>(m_VAO->GetVertexBuffer(0)->GetLength()));
    }

    void RebuildFromData( IRenderer& renderer, const Plot::CurveData& data, bool doBufferReload )
    {
        if (m_VAO == nullptr)
            Init(renderer);

        if (doBufferReload)
			m_VAO->GetVertexBuffer(0)->SetData(data.GetData());

        m_uniforms->Commit([&](AT2::IUniformsWriter& writer) {
            writer.Write("u_BoundsX", glm::vec2(data.GetCurveBounds().MinBound.x, data.GetCurveBounds().MaxBound.x));
            writer.Write("u_NumberOfPoints", (glm::uint32_t)data.GetData().size());
            writer.Write("u_Color", data.GetColor());
        });
    }

    void SetProjectionMatrix(const glm::mat4& matProj) { m_projectionMatrix = matProj; }

private:
    void Init(IRenderer& renderer)
    {
        auto& rf = renderer.GetResourceFactory();

        m_VAO = rf.CreateVertexArray();
        m_VAO->SetAttributeBinding(0, rf.CreateBuffer(VertexBufferType::ArrayBuffer), AT2::BufferDataTypes::Float);


        m_uniforms = std::make_shared<UniformContainer>();
    }

private:
    std::shared_ptr<IVertexArray> m_VAO;
    std::shared_ptr<IUniformContainer> m_uniforms;
    glm::mat4 m_projectionMatrix {1.0};
};


void PlotRenderer::Draw(IRenderer& renderer)
{
    if (m_uiShader == nullptr)
        Init(renderer);

    auto& stateManager = renderer.GetStateManager();

    stateManager.BindShader(m_uiShader);
    m_uniformBuffer->Bind(stateManager);
    LinesHelper::Draw(renderer);


    stateManager.BindShader(m_curveShader);
    PrepareData(renderer);

    for (const auto& pair : m_curves)
        pair.second->Draw(renderer);
}

void PlotRenderer::PrepareData( IRenderer& renderer )
{
    if (auto controlPtr = m_Control.lock())
    {
        const auto& observingRange = controlPtr->GetObservingZone();
        m_projectionMatrix = glm::ortho(observingRange.MinBound.x, observingRange.MaxBound.x, observingRange.MinBound.y,
                                        observingRange.MaxBound.y);

        controlPtr->EnumerateCurves([&](const std::string_view name, const Plot::CurveData& data, bool isInvalidated) {
            const auto [drawableIt, wasInserted] = m_curves.try_emplace(std::string(name), std::make_shared<CurveDrawable>());
            drawableIt->second->RebuildFromData(renderer, data, isInvalidated || wasInserted);

            return true;
        });

        for (auto& pair : m_curves)
            pair.second->SetProjectionMatrix(m_projectionMatrix);

        UpdateCanvasGeometry(observingRange);
    }

    m_uniformBuffer->SetUniform("u_matProjection", m_projectionMatrix);
}

void PlotRenderer::UpdateCanvasGeometry(const AABB2d& observingRange)
{
    Clear();

    //axises
    AddLine(glm::vec2(observingRange.MinBound.x, 0.0), glm::vec2(observingRange.MaxBound.x, 0.0));
    AddLine(glm::vec2(0.0, observingRange.MinBound.y), glm::vec2(0.0, observingRange.MaxBound.y));

    //let's draw a coordinate grid here
    const glm::vec2 range = observingRange.MaxBound - observingRange.MinBound;
    const glm::vec2 exp = glm::floor(glm::log(range) / glm::log(glm::vec2(10.0f)));

    const glm::vec2 step = glm::pow(glm::vec2(10.0f), exp - glm::vec2(1));
    const glm::vec2 firstMark = glm::floor(observingRange.MinBound / step) * step;

    for (auto x = firstMark.x; x <= observingRange.MaxBound.x; x += step.x)
        AddLine(glm::vec2(x, observingRange.MinBound.y), glm::vec2(x, observingRange.MaxBound.y),
                glm::vec4(1.0, 1.0, 1.0, 0.3));

    for (auto y = firstMark.y; y <= observingRange.MaxBound.y; y += step.y)
        AddLine(glm::vec2(observingRange.MinBound.x, y), glm::vec2(observingRange.MaxBound.x, y),
                glm::vec4(1.0, 1.0, 1.0, 0.3));
}

void PlotRenderer::Init(const IVisualizationSystem& renderer)
{
    m_uiShader = renderer.GetResourceFactory().CreateShaderProgramFromFiles(
        {"resources//shaders//simple.vs.glsl", "resources//shaders//simple.fs.glsl"});

    m_curveShader = renderer.GetResourceFactory().CreateShaderProgramFromFiles(
        {"resources//shaders//curve.vs.glsl", "resources//shaders//curve.fs.glsl"});


    m_uniformBuffer = std::make_shared<UniformContainer>();
}

void WindowRenderer::Draw(IRenderer& renderer)
{
    if (const auto control = m_Control.lock())
    {
        const auto screenAABB = control->GetScreenPosition();

        //it's pretty costly and not so important, but looks nice. Until it's not bottleneck, let's continue
        const auto texture =
            renderer.GetResourceFactory().CreateTextureFromFramebuffer(screenAABB.MinBound, screenAABB.GetSize());

        m_Mesh->GetOrCreateDefaultMaterial().Commit([&](AT2::IUniformsWriter& writer) {
            writer.Write("u_BackgroundTexture", texture);
            writer.Write("u_ScreenAABB", glm::vec4(screenAABB.MinBound, screenAABB.MaxBound));
            writer.Write("u_Color", m_Color);
            writer.Write("u_BorderThickness", m_borderThickness);
            writer.Write("u_BlurDirection", m_blurDirection);
        });
        Utils::MeshRenderer::DrawMesh(renderer, *m_Mesh, m_Mesh->Shader);
    }
}
