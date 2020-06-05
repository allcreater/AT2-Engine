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
        RenderVisitor rv{ renderer, camera };
        scene.GetRoot().Accept(rv);
    }

private:
    const Camera& camera;
    MeshNode* active_mesh = nullptr;

    IRenderer& renderer;
    MatrixStack transforms;
};