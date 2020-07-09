#pragma once

#include "AT2.h"
#include "UniformContainer.h"

namespace AT2
{

    struct MeshChunk
    {
        MeshChunk() = default;
        MeshChunk(Primitives::Primitive type, unsigned startElement, unsigned count, unsigned baseVertex = 0) :
            Type(type), StartElement(startElement), Count(count), BaseVertex(baseVertex)
        {
        }

        Primitives::Primitive Type;
        // First vertex or vertex index (if index buffer present)
        unsigned int StartElement;
        //Number of vertices or indices to be rendered
        unsigned int Count;

        unsigned int BaseVertex;
    };

    struct SubMesh
    {
        SubMesh() = default;
        SubMesh(std::vector<MeshChunk> primitives, int matIndex = 0, std::string name = "") :
            MaterialIndex(matIndex >= 0 ? matIndex
                                        : throw std::invalid_argument("matIndex must be great or equal to zero!")),
            Name(std::move(name)), Primitives(std::move(primitives))
        {
        }

        SubMesh(SubMesh&&) = default;
        SubMesh& operator=(SubMesh&&) = default;
        SubMesh(const SubMesh&) = default;
        SubMesh& operator=(const SubMesh&) = default;

        int MaterialIndex = 0;
        std::string Name;
        std::vector<MeshChunk> Primitives;
    };

    struct Mesh
    {
        //Mesh() = default;
        //Mesh(Mesh&&) = default;
        //Mesh& operator=(Mesh&&) = default;

        //Mesh(const Mesh&) = delete;
        //Mesh& operator=(const Mesh&) = delete;

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

} // namespace AT2