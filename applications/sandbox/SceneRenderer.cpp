#include "SceneRenderer.h"

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

        mesh->UniformBuffer->Bind();
    }
    else if (auto submesh = dynamic_cast<DrawableNode*>(&node))
    {
        if (!active_mesh)
            return;

        stateManager.BindTextures(submesh->Textures);

        submesh->UniformBuffer->SetUniform("u_matModel", transforms.getModelView());
        submesh->UniformBuffer->SetUniform("u_matNormal",
                                           glm::mat3(glm::transpose(
                                               glm::inverse(camera.getView() * transforms.getModelView()))));
        submesh->UniformBuffer->Bind();

        for (auto& primitive : submesh->Primitives)
            primitive->Draw();
    }
}

void RenderVisitor::UnVisit(Node& node)
{
    transforms.popModelView();
}
