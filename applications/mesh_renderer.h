#include <AT2/Mesh.h>

namespace AT2::Utils
{
    struct MeshRenderer
    {
        static void DrawSubmesh(IRenderer& renderer, const Mesh& mesh, const SubMesh& subMesh, size_t numInstances = 1)
        {
            auto& stateManager = renderer.GetStateManager();

            if (!mesh.Materials.empty())
                mesh.Materials.at(subMesh.MaterialIndex)->Bind(stateManager);

            for (const auto& primitive : subMesh.Primitives)
                renderer.Draw(primitive.Type, primitive.StartElement, primitive.Count, static_cast<int>(numInstances),
                               primitive.BaseVertex);
        }

        static void DrawMesh(IRenderer& renderer, const Mesh& mesh, const std::shared_ptr<IShaderProgram>& program)
        {
            auto& stateManager = renderer.GetStateManager();

            if (program)
                stateManager.BindShader(program);

            stateManager.BindVertexArray(mesh.VertexArray);

            for (const auto& submesh : mesh.SubMeshes)
                DrawSubmesh(renderer, mesh, submesh);
        }
    };

} // namespace AT2::Utils