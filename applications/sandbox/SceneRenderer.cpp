#include "SceneRenderer.h"

#include "AT2/OpenGL/GlDrawPrimitive.h"

//using namespace AT2;

RenderVisitor::RenderVisitor(IRenderer& renderer, const Camera& camera):
    camera(camera),
    renderer(renderer)
{
    //transforms.reset(camera.getView(), camera.getProjection());
}

void RenderVisitor::Visit(Node& node)
{
    transforms.pushModelView(node.GetTransform());

    auto& stateManager = renderer.GetStateManager();

    if (auto mesh = dynamic_cast<MeshNode*>(&node))
    {
        active_mesh = mesh; //TODO: some kind of RenderContext?

        stateManager.BindShader(mesh->Shader);
        stateManager.BindVertexArray(mesh->VertexArray);

        if (mesh->UniformBuffer)
            mesh->UniformBuffer->Bind();
    }
    else if (auto submesh = dynamic_cast<DrawableNode*>(&node))
    {
        if (!active_mesh)
            return;

        stateManager.BindTextures(submesh->Textures);

        if (submesh->UniformBuffer)
        {
            //submesh->UniformBuffer->SetUniform("u_time", time);
            submesh->UniformBuffer->SetUniform("u_matModel", transforms.getModelView());
            submesh->UniformBuffer->SetUniform("u_matNormal", glm::mat3(transpose(inverse(camera.getView() * transforms.getModelView()))));
            submesh->UniformBuffer->Bind();
        }

        for (auto& primitive : submesh->Primitives)
            primitive->Draw();
    }
}

void RenderVisitor::UnVisit(Node& node)
{
    transforms.popModelView();
}

std::shared_ptr<MeshNode> MakeTerrain(IRenderer& renderer, std::shared_ptr<IShaderProgram> program, int segX, int segY)
{
    assert(segX < 1024 && segY < 1024);

    std::vector<glm::vec2> texCoords(segX * segY * 4);//TODO! GlVertexBuffer - take iterators!

    for (size_t j = 0; j < segY; ++j)
    for (size_t i = 0; i < segX; ++i)
    {
        const auto num = (i + j * segX) * 4;
        texCoords[num] = glm::vec2(float(i) / segX, float(j) / segY);
        texCoords[num + 1] = glm::vec2(float(i + 1) / segX, float(j) / segY);
        texCoords[num + 2] = glm::vec2(float(i + 1) / segX, float(j + 1) / segY);
        texCoords[num + 3] = glm::vec2(float(i) / segX, float(j + 1) / segY);
    }


    auto& rf = renderer.GetResourceFactory();
    auto vao = rf.CreateVertexArray();
    vao->SetVertexBuffer(1, rf.CreateVertexBuffer(VertexBufferType::ArrayBuffer, AT2::BufferDataTypes::Vec2, texCoords.size() * sizeof(glm::vec2), texCoords.data()));


    auto mesh = std::make_shared<MeshNode>();
    mesh->SetName("Terrain");
    mesh->Shader = program;
    mesh->UniformBuffer = program->CreateAssociatedUniformStorage();
    mesh->VertexArray = vao;

    auto drawable = std::make_shared<DrawableNode>();
    drawable->SetName("Terrain drawable");
    //drawable->SetTransform(ConvertMatrix(node->mTransformation));
    drawable->UniformBuffer = mesh->Shader->CreateAssociatedUniformStorage();
    drawable->Primitives.push_back(std::make_unique<GlDrawArraysPrimitive>(AT2::GlDrawPrimitiveType::Patches, 0, texCoords.size()));

    mesh->AddChild(std::move(drawable));

    return mesh;
}
