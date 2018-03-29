#include "UI.h"

using namespace AT2;
using namespace AT2::UI;

//class RenderablePlot : public Plot, public IDrawable
//{
//public:
//	RenderablePlot(std::string_view name, const glm::ivec2& size) : Plot(name, size)
//	{
//	}
//
//	void Draw(const std::shared_ptr<IRenderer>& renderer) override
//	{
//		auto& stateManager = renderer->GetStateManager();
//
//		stateManager.BindShader(m_shader);
//		//stateManager.BindTextures(Textures);
//		stateManager.BindVertexArray(m_uiVAO);
//		
//
//		m_uniformBuffer->Bind();
//
//		for (auto primitive : Primitives)
//			primitive->Draw();
//	}
//
//protected:
//	void PrepareData(const std::shared_ptr<IRenderer>& renderer)
//	{
//		for (auto& data : m_curvesData)
//		{
//			data.second.
//		}
//
//	}
//
//	void Init(const std::shared_ptr<IRenderer>& renderer)
//	{
//		glm::vec3 positions[] = { glm::vec3(-1.0, -1.0, -1.0), glm::vec3(1.0, -1.0, -1.0), glm::vec3(1.0, 1.0, -1.0), glm::vec3(-1.0, 1.0, -1.0) };
//		glm::uint indices[] = { 0, 1, 2, 0, 2, 3 };
//
//		auto& rf = renderer->GetResourceFactory();
//
//		m_uiVAO = rf.CreateVertexArray();
//		m_uiVAO->SetVertexBuffer(1, rf.CreateVertexBuffer(AT2vbt::ArrayBuffer, AT2::BufferDataTypes::Vec3, 4 * sizeof(glm::vec3), positions));
//		m_uiVAO->SetIndexBuffer(rf.CreateVertexBuffer(AT2vbt::IndexBuffer, AT2::BufferDataTypes::UInt, 6 * sizeof(glm::uint), indices));
//
//		auto drawable = std::make_shared<AT2::MeshDrawable>();
//		drawable->Primitives.push_back(new AT2::GlDrawElementsPrimitive(AT2::GlDrawPrimitiveType::Triangles, 6, AT2::GlDrawElementsPrimitive::IndicesType::UnsignedInt, 0));
//		drawable->VertexArray = m_uiVAO;
//	}
//
//private:
//	struct CurveDataStatus
//	{
//		size_t PreviousSize;
//		std::shared_ptr<IVertexArray> VAO;
//		std::unique_ptr<IDrawPrimitive> DrawPrimitive;
//	};
//
//	std::shared_ptr<IVertexArray> m_uiVAO;
//	std::shared_ptr<IShaderProgram> m_shader;
//	std::shared_ptr<IUniformContainer> m_uniformBuffer;
//	std::map<std::string, CurveDataStatus> m_CurveDataStatus;
//};