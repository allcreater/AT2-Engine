#include "MeshLoader.h"


#include <assimp/Importer.hpp>
#include <assimp/pbrmaterial.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <filesystem>
#include <map>
#include <utility>

#include "TextureLoader.h"


using namespace std::literals;

using namespace AT2;
using namespace AT2::Scene;
using namespace AT2::Resources;

namespace
{
    class MeshBuilder
    {
    public:
        MeshBuilder(std::shared_ptr<IRenderer> _renderer, const aiScene* scene) :
            m_scene(scene), m_renderer(std::move(_renderer))
        {
        }

    public:
        std::shared_ptr<Node> Build()
        {
            assert(m_indicesVec.empty());
            BuildVAO();

            m_buildingMesh->Materials.reserve(m_scene->mNumMaterials);
            for (size_t i = 0; i < m_scene->mNumMaterials; ++i)
                m_buildingMesh->Materials.push_back(TranslateMaterial(m_scene->mMaterials[i]));

            auto rootNode = std::make_shared<Node>("Root");
            TraverseNode(m_scene->mRootNode, *rootNode);

            return rootNode;
        }

    protected:
        std::shared_ptr<Mesh> m_buildingMesh = std::make_shared<Mesh>();
        std::filesystem::path m_scenePath;
        Assimp::Importer m_importer;
        const aiScene* m_scene;
        std::shared_ptr<IRenderer> m_renderer;


        std::vector<glm::vec3> m_verticesVec;
        std::vector<glm::vec3> m_texCoordVec;
        std::vector<glm::vec3> m_normalsVec;
        std::vector<std::uint32_t> m_indicesVec;

    protected:
        void AddMesh(const aiMesh* mesh)
        {
            const auto vertexOffset = static_cast<std::uint32_t>(m_verticesVec.size());
            const auto previousIndexOffset = static_cast<unsigned>(m_indicesVec.size());

            m_verticesVec.insert(m_verticesVec.end(), reinterpret_cast<glm::vec3*>(mesh->mVertices),
                                 reinterpret_cast<glm::vec3*>(mesh->mVertices) + mesh->mNumVertices);
            m_texCoordVec.insert(m_texCoordVec.end(), reinterpret_cast<glm::vec3*>(mesh->mTextureCoords[0]),
                                 reinterpret_cast<glm::vec3*>(mesh->mTextureCoords[0]) + mesh->mNumVertices);
            m_normalsVec.insert(m_normalsVec.end(), reinterpret_cast<glm::vec3*>(mesh->mNormals),
                                reinterpret_cast<glm::vec3*>(mesh->mNormals) + mesh->mNumVertices);

            for (size_t j = 0; j < mesh->mNumFaces; ++j)
            {
                const aiFace& face = mesh->mFaces[j];
                assert(face.mNumIndices == 3);

                m_indicesVec.push_back(face.mIndices[0] + vertexOffset);
                m_indicesVec.push_back(face.mIndices[1] + vertexOffset);
                m_indicesVec.push_back(face.mIndices[2] + vertexOffset);
            }

            m_buildingMesh->SubMeshes.emplace_back(
                std::vector<MeshChunk> {MeshChunk {Primitives::Triangles {}, previousIndexOffset, mesh->mNumFaces * 3}},
                mesh->mMaterialIndex, mesh->mName.C_Str());
        }

        void BuildVAO()
        {
            for (unsigned i = 0; i < m_scene->mNumMeshes; ++i)
                AddMesh(m_scene->mMeshes[i]);

            auto& rf = m_renderer->GetResourceFactory();

            auto vao = MakeVertexArray(rf, std::make_pair(1u, std::cref(m_verticesVec)),
                                       std::make_pair(2u, std::cref(m_texCoordVec)),
                                       std::make_pair(3u, std::cref(m_normalsVec)));
            vao->SetIndexBuffer(rf.MakeVertexBufferFrom(VertexBufferType::IndexBuffer, m_indicesVec),
                                BufferDataType::UInt);

            m_buildingMesh->VertexArray = vao;
        }

        std::unique_ptr<IUniformContainer> TranslateMaterial(const aiMaterial* material)
        {
            std::unique_ptr<IUniformContainer> container = std::make_unique<UniformContainer>();

            constexpr std::tuple<aiTextureType, unsigned, std::string_view> knownTextureFlavors[] = {
                {aiTextureType_DIFFUSE, 0, "u_texAlbedo"sv},
                {AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_TEXTURE, "u_texAlbedo"sv},
                {aiTextureType_NORMALS, 0, "u_texNormalMap"sv},
                {aiTextureType_HEIGHT, 0, "u_texNormalMap"sv},
                {AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, "u_texAoRoughnessMetallic"sv}};

            auto loadTexture = [=](const char* path) -> TextureRef {
                if (const aiTexture* embeddedTexture = m_scene->GetEmbeddedTexture(path))
                {
                    if (embeddedTexture->mHeight)
                        throw AT2TextureException("reading raw texture from memory not implemented yet");

                    return TextureLoader::LoadTexture(m_renderer,
                                                      std::span {reinterpret_cast<const std::byte*>(embeddedTexture->pcData), embeddedTexture->mWidth});
                }

                //it's not embedded
                return TextureLoader::LoadTexture(m_renderer, path);
            };

            std::map<str, std::shared_ptr<ITexture>, std::less<>> textures;
            for (auto [type, index, name] : knownTextureFlavors)
            {
                if (textures.find(name) != textures.end())
                    continue;

                if (aiString path; material->GetTexture(type, index, &path) == aiReturn_SUCCESS && path.length > 0)
                {
                    if (auto texture = loadTexture(path.C_Str()))
                    {
                        textures.emplace(name, texture);
                        container->SetUniform(str {name}, texture);
                    }
                }
            }

            return container;
        }

        void TraverseNode(const aiNode* node, Node& baseNode)
        {
            baseNode.SetTransform(ConvertMatrix(node->mTransformation));
            for (unsigned i = 0; i < node->mNumMeshes; i++)
            {
                const unsigned meshIndex = node->mMeshes[i];
                //TODO: don't multiply components when materials are the same
                baseNode.addComponent(std::make_unique<MeshComponent>(m_buildingMesh, std::vector {meshIndex}));
            }

            for (size_t i = 0; i < node->mNumChildren; i++)
            {
                auto* children = node->mChildren[i];

                auto subNode = std::make_shared<Node>();
                subNode->SetName(children->mName.C_Str());
                baseNode.AddChild(subNode);

                TraverseNode(children, *subNode);
            }
        }

        static glm::mat4 ConvertMatrix(const aiMatrix4x4& _t) { return glm::transpose(glm::make_mat4(&_t.a1)); }
    };

    constexpr auto flags = static_cast<unsigned>(
        aiProcess_GenSmoothNormals | //can't be specified with aiProcess_GenNormals
        aiProcess_FindInstances | aiProcess_FindDegenerates | aiProcess_FindInvalidData | aiProcess_CalcTangentSpace |
        aiProcess_ValidateDataStructure | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph |
        aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality | aiProcess_LimitBoneWeights |
        aiProcess_RemoveRedundantMaterials | aiProcess_SplitLargeMeshes | aiProcess_Triangulate |
        aiProcess_GenUVCoords | aiProcess_GenBoundingBoxes | aiProcess_SplitByBoneCount | aiProcess_SortByPType |
        aiProcess_FlipUVs);

}; // namespace

std::shared_ptr<Node> MeshLoader::LoadNode(std::shared_ptr<IRenderer> renderer, const str& filename)
{
    Assimp::Importer importer;
    const auto* scene = importer.ReadFile(filename, flags);

    MeshBuilder builder {std::move(renderer), scene};
    return builder.Build();
}
