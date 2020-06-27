#pragma once

#include "AT2.h"
#include "UniformContainer.h"

namespace AT2 {

struct MeshChunk
{
    MeshChunk() = default;
    MeshChunk(Primitives::Primitive type, unsigned startElement, unsigned count, unsigned baseVertex = 0) :
        Type(type), StartElement(startElement), Count(count), BaseVertex(baseVertex)
    {
    }

    Primitives::Primitive Type;
    // First vertex or vertex index (if index buffer present)
    unsigned int StartElement = 0;
    //Number of vertices or indices to be rendered
    unsigned int Count = 0;


    unsigned int BaseVertex = 0;
};

struct SubMesh
{
    //SubMesh() = default;
    //SubMesh(SubMesh&&) = default;
    //SubMesh& operator=(SubMesh&&) = default;

    //SubMesh(const SubMesh&) = delete;
    //SubMesh& operator=(const SubMesh&) = delete;

    std::string Name;

    TextureSet Textures;
    std::shared_ptr<IUniformContainer> UniformBuffer;
    std::vector<MeshChunk> Primitives;
};

struct Mesh
{
    //Mesh() = default;
    //Mesh(Mesh&&) = default;
    //Mesh& operator=(Mesh&&) = default;

    //Mesh(const Mesh&) = delete;
    //Mesh& operator=(const Mesh&) = delete;

    std::shared_ptr<IUniformContainer> GetOrCreateUniformBuffer()
    {
        if (!UniformBuffer)
            UniformBuffer = std::make_shared<UniformContainer>();

        return UniformBuffer;
    }

    std::string Name;
    std::shared_ptr<IShaderProgram> Shader;
    std::shared_ptr<IVertexArray> VertexArray;
    std::shared_ptr<IUniformContainer> UniformBuffer;

    std::vector<SubMesh> Submeshes; //TODO: move Mesh and Submesh from scene so that nodes could be builded by Mesh
};

}