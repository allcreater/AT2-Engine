#pragma once

#include <AT2/Scene.h>
#include <AT2/matrix_stack.h>

using namespace AT2;



struct RenderVisitor : NodeVisitor
{
    RenderVisitor(IRenderer& renderer, const Camera& camera);

    virtual void Visit(Node& node);

    virtual void UnVisit(Node& node);

    static void RenderScene(Scene& scene, IRenderer& renderer, const Camera& camera)
    {
        RenderVisitor rv { renderer, camera };
        scene.GetRoot().Accept(rv);
    }

private:
    const Camera& camera;
    IRenderer& renderer;

    MatrixStack transforms;
    MeshNode* active_mesh = nullptr;

};

std::shared_ptr<MeshNode> MakeTerrain(IRenderer& renderer, std::shared_ptr<IShaderProgram> program, int segX, int segY);