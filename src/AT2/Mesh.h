#pragma once

#include "AT2.h"
#include "UniformContainer.h"

namespace AT2
{

    struct MeshChunk
    {
        Primitives::Primitive Type = Primitives::Triangles{};
        // First vertex or vertex index (if index buffer present)
        unsigned int StartElement = 0;
        // Number of vertices or indices to be rendered
        unsigned int Count = 0;
        int BaseVertex = 0;
    };

    struct SubMesh
    {
        SubMesh() = default;
        SubMesh(std::vector<MeshChunk> primitives, int matIndex = 0, std::string name = "") :
            MaterialIndex(matIndex >= 0 ? static_cast<unsigned int>(matIndex)
                                        : throw std::invalid_argument("matIndex must be more or equal than zero!")),
            Name(std::move(name)), Primitives(std::move(primitives))
        {
        }

        unsigned int MaterialIndex = 0;
        std::string Name;
        std::vector<MeshChunk> Primitives;
    };

    struct Mesh
    {
        IUniformContainer& GetOrCreateDefaultMaterial()
        {
            if (Materials.empty())
                Materials.push_back(std::make_unique<UniformContainer>());

            return *Materials[0];
        }

        std::string Name;
        std::shared_ptr<IShaderProgram> Shader;
        std::shared_ptr<IVertexArray> VertexArray;

        std::vector<std::unique_ptr<IUniformContainer>> Materials;
        std::vector<SubMesh> SubMeshes; //TODO: move Mesh and Submesh from scene so that nodes could be builded by Mesh
    };

    using MeshRef = std::shared_ptr<Mesh>;
    using ConstMeshRef = std::shared_ptr<Mesh>;

} // namespace AT2