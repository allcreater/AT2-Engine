#pragma once

#include <Mesh.h>

namespace AT2::Utils::MeshRenderer
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

	static void DrawMesh(IRenderer& renderer, const Mesh& mesh, const std::shared_ptr<IPipelineState>& pipelineState)
	{
        auto& stateManager = renderer.GetStateManager();

	    if (pipelineState)
	        stateManager.ApplyPipelineState(pipelineState);

	    stateManager.BindVertexArray(mesh.VertexArray);

	    for (const auto& submesh : mesh.SubMeshes)
	        DrawSubmesh(renderer, mesh, submesh);
	}

} // namespace AT2::Utils
