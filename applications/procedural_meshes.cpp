#include "procedural_meshes.h"
#include <Scene/Scene.h>

namespace AT2::Utils
{
    std::unique_ptr<Scene::Node> MakeTerrain(const IVisualizationSystem& renderer, glm::uvec2 numPatches)
    {
        assert(numPatches.x < 1024 && numPatches.y < 1024);

        std::vector<glm::vec2> texCoords(4 * numPatches.x * numPatches.y); //TODO! GlVertexBuffer - take iterators!

        for (size_t j = 0; j < numPatches.y; ++j)
            for (size_t i = 0; i < numPatches.x; ++i)
            {
                const auto num = (i + j * numPatches.x) * 4;
                texCoords[num] = glm::vec2(float(i) / numPatches.x, float(j) / numPatches.y);
                texCoords[num + 1] = glm::vec2(float(i + 1) / numPatches.x, float(j) / numPatches.y);
                texCoords[num + 2] = glm::vec2(float(i + 1) / numPatches.x, float(j + 1) / numPatches.y);
                texCoords[num + 3] = glm::vec2(float(i) / numPatches.x, float(j + 1) / numPatches.y);
            }


        auto& rf = renderer.GetResourceFactory();
        auto vao = rf.CreateVertexArray();
        vao->SetAttributeBinding(1, rf.MakeBufferFrom(VertexBufferType::ArrayBuffer, texCoords), BufferDataTypes::Vec2);


        auto mesh = std::make_shared<Mesh>();
        mesh->Name = "Terrain";
        mesh->VertexArray = vao;

        SubMesh subMesh;
        subMesh.Primitives.push_back({Primitives::Patches {4}, 0u, static_cast<unsigned int>(texCoords.size())});

        mesh->SubMeshes.push_back(std::move(subMesh));


        auto node = std::make_unique<Scene::Node>("Terrain drawable");
        node->createUniqueComponent<Scene::MeshComponent>(std::move(mesh), std::vector {0u});

        return node;
    }

    std::unique_ptr<Mesh> MakeSphere(const IVisualizationSystem& renderer, glm::uvec2 numPatches)
    {
        assert(numPatches.x <= 1024 && numPatches.y <= 512);

        std::vector<glm::vec3> normals;
        normals.reserve(static_cast<size_t>(numPatches.x) * numPatches.y);
        std::vector<uint32_t> indices;
        indices.reserve(static_cast<size_t>(numPatches.x) * numPatches.y * 6);

        for (uint32_t j = 0; j < numPatches.y; ++j)
        {
            const double angV = j * glm::pi<double>() / (numPatches.y - 1);
            for (uint32_t i = 0; i < numPatches.x; ++i)
            {
                const double angH = i * glm::pi<double>() * 2 / numPatches.x;

                normals.emplace_back(sin(angV) * cos(angH), sin(angV) * sin(angH), cos(angV));
            }
        }

        for (uint32_t j = 0; j < numPatches.y - 1; ++j)
        {
            const auto nj = j + 1;
            for (uint32_t i = 0; i < numPatches.x; ++i)
            {
                const int ni = (i + 1) % numPatches.x;

                indices.push_back(j * numPatches.x + i);
                indices.push_back(j * numPatches.x + ni);
                indices.push_back(nj * numPatches.x + ni);
                indices.push_back(j * numPatches.x + i);
                indices.push_back(nj * numPatches.x + ni);
                indices.push_back(nj * numPatches.x + i);
            }
        }

        auto& rf = renderer.GetResourceFactory();

        auto mesh = std::make_unique<Mesh>();

        mesh->VertexArray = MakeVertexArray(rf, std::make_pair(1u, std::cref(normals)));
        mesh->VertexArray->SetIndexBuffer(rf.MakeBufferFrom(VertexBufferType::IndexBuffer, indices),
                                          BufferDataType::UInt);


        //don't know how to make it better
        SubMesh subMesh;
        subMesh.Primitives.push_back({Primitives::Triangles {}, 0, static_cast<unsigned int>(indices.size())});
        mesh->SubMeshes.push_back(std::move(subMesh));

        return mesh;
    }

    std::unique_ptr<Mesh> MakeFullscreenQuadMesh(const IVisualizationSystem& renderer)
    {
        constexpr std::array positions = {glm::vec3(-1.0, -1.0, -1.0), glm::vec3(1.0, -1.0, -1.0),
                                        glm::vec3(1.0, 1.0, -1.0), glm::vec3(-1.0, 1.0, -1.0)};

        constexpr std::array indices = {0, 1, 2, 0, 2, 3};

        auto& rf = renderer.GetResourceFactory();
        auto vao = MakeVertexArray(rf, std::make_pair(1u, std::cref(positions)));
        vao->SetIndexBuffer(rf.MakeBufferFrom(VertexBufferType::IndexBuffer, indices), BufferDataType::UInt);

        
        SubMesh subMesh;
        subMesh.Primitives.push_back({Primitives::Triangles {}, 0, indices.size()});

        auto mesh = std::make_unique<Mesh>();
        mesh->VertexArray = vao;
        mesh->SubMeshes.push_back(std::move(subMesh));

        return mesh;
    }

} // namespace AT2::Utils