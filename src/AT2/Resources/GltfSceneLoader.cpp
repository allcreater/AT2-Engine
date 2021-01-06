#include "GltfSceneLoader.h"

#include <filesystem>


#include "../BufferMapperGuard.h"

#include "TextureLoader.h"

using namespace AT2;
using namespace AT2::Resources;

using namespace std::literals;

#include <fx/gltf.h>
#include <ranges>
#include <bit>

namespace
{
    //from gltf viewer example
    std::optional<BufferBindingParams> TranslateDataType(fx::gltf::Accessor const& accessor) noexcept
    {
        BufferBindingParams result;
        result.Count = 1;

        switch (accessor.componentType)
        {
        case fx::gltf::Accessor::ComponentType::Byte: 
            result.Type = BufferDataType::Byte;
            result.Stride = 1;
            break;
        case fx::gltf::Accessor::ComponentType::UnsignedByte:
            result.Type = BufferDataType ::UByte;
            result.Stride = 1;
            break;
        case fx::gltf::Accessor::ComponentType::Short:
            result.Type = BufferDataType::Short;
            result.Stride = 2;
            break;
        case fx::gltf::Accessor::ComponentType::UnsignedShort:
            result.Type = BufferDataType ::UShort;
            result.Stride = 2;
            break;
        case fx::gltf::Accessor::ComponentType::Float:
            result.Type = BufferDataType ::Float;
            result.Stride = 4;
            break;
        case fx::gltf::Accessor::ComponentType::UnsignedInt:
            result.Type = BufferDataType ::UInt;
            result.Stride = 4;
            break;
        case fx::gltf::Accessor::ComponentType::None:
            return {};
        }
        
        switch (accessor.type)
        {
        case fx::gltf::Accessor::Type::Mat2: result.Stride *= 4; break;
        case fx::gltf::Accessor::Type::Mat3: result.Stride *= 9; break;
        case fx::gltf::Accessor::Type::Mat4: result.Stride *= 16; break;
        case fx::gltf::Accessor::Type::Scalar: break;
        case fx::gltf::Accessor::Type::Vec2:
            result.Count = 2;
            result.Stride *= 2;
            break;
        case fx::gltf::Accessor::Type::Vec3:
            result.Count = 3;
            result.Stride *= 3;
            break;
        case fx::gltf::Accessor::Type::Vec4:
            result.Count = 4;
            result.Stride *= 4;
            break;
        case fx::gltf::Accessor::Type::None:
            return {};
        }
 
        return result;
    }

    constexpr TextureWrapMode TranslateWrappingMode(fx::gltf::Sampler::WrappingMode wrappingMode)
    {
        using WrappingMode = fx::gltf::Sampler::WrappingMode;

        switch (wrappingMode)
        {
            case WrappingMode::ClampToEdge: return TextureWrapMode::ClampToEdge;
            case WrappingMode::Repeat: return TextureWrapMode::Repeat;
            case WrappingMode::MirroredRepeat: return TextureWrapMode::MirroredRepeat;
        }

        return TextureWrapMode::ClampToBorder;
    }

    class Loader
    {
        std::shared_ptr<IRenderer> m_renderer;
        fx::gltf::Document m_document;

        using SubmeshGroup = std::vector<MeshRef>;
        std::vector<SubmeshGroup> m_meshes;
        std::vector<std::shared_ptr<ITexture>> m_textures;
        std::filesystem::path m_currentPath;

        std::shared_ptr<ITexture> m_normalMapPlaceholder;

    public:
        Loader(std::shared_ptr<IRenderer> renderer, const str& sv) :
            m_renderer(std::move(renderer)), m_document(fx::gltf::LoadFromText(sv)),
            m_currentPath(sv)
        {
            m_currentPath.remove_filename();

            m_normalMapPlaceholder =
                m_renderer->GetResourceFactory().CreateTexture(Texture2D {{1, 1}},
                                                                               AT2::TextureFormats::RGBA8);
            std::array<glm::u8, 4> color = {127, 127, 255, 255};
            m_normalMapPlaceholder->SubImage2D({}, {1, 1}, 0, TextureFormats::RGBA8, color.data());
        }

        std::shared_ptr<Node> BuildScene()
        {
            Log::Info() << "Building scene graph..." << std::endl;
            LoadResources();

            if (m_document.scene >= 0 && static_cast<size_t>(m_document.scene) < m_document.scenes.size())
            {
                Log::Info() << "Building scene graph..." << std::endl;

                const auto& scene = m_document.scenes[m_document.scene];

                auto sceneRoot = std::make_shared<Node>(scene.name + " root"s);
                for (const auto nodeIndex : scene.nodes)
                {
                    BuildSceneGraph(m_document.nodes[nodeIndex], *sceneRoot);
                }

                return sceneRoot;
            }

            Log::Info() << "Scene graph is not available, loading as individual meshes" << std::endl;
            auto root = std::make_shared<Node>("Model root"s);
            for (size_t meshIndex = 0; meshIndex < m_meshes.size(); ++meshIndex)
            {
                auto childNode = std::make_shared<Node>();
                root->AddChild(childNode);
                PopulateSubmeshes(*childNode, meshIndex);
            }


        }

        void PopulateSubmeshes(AT2::Node& node, size_t meshIndex)
        {
            node.SetName("Mesh group"s + m_document.meshes[meshIndex].name);
            for (const auto& submesh : m_meshes[meshIndex])
            {
                auto meshNode = std::make_shared<MeshNode>(submesh, submesh->Name);
                meshNode->AddChild(std::make_shared<DrawableNode>(0));

                node.AddChild(std::move(meshNode));
            }
        }

        void BuildSceneGraph(const fx::gltf::Node& node, AT2::Node& baseNode)
        {
            auto currentNode = std::make_shared<Node>();
            currentNode->SetTransform(std::bit_cast<glm::mat4>(node.matrix));

            baseNode.AddChild(currentNode);
            
            if (node.mesh >= 0)
                PopulateSubmeshes(*currentNode, static_cast<size_t>(node.mesh));

            if (const size_t cameraIndex = node.camera; node.camera >= 0)
            {
                
            }

            for (const auto nodeIndex : node.children)
            {
                assert(nodeIndex > 0);
                const auto& childNode = m_document.nodes[nodeIndex];

                BuildSceneGraph(childNode, *currentNode);
            }
        }

    private:
        struct BufferDataInfo
        {
            BufferBindingParams bindingParams;
            uint32_t count;
            std::span<const std::byte> data;
        };

        BufferDataInfo GetData(unsigned attribIndex)
        {
            const auto& accessor = m_document.accessors[attribIndex];
            const auto& bufferView = m_document.bufferViews[accessor.bufferView];
            const auto& buffer = m_document.buffers[bufferView.buffer];


            const auto dataType = TranslateDataType(accessor);
            if (!dataType.has_value())
                throw std::logic_error("unsupported data format");

            return {*dataType, accessor.count,
                    std::as_bytes(std::span {buffer.data})
                        .subspan(static_cast<uint64_t>(bufferView.byteOffset) + accessor.byteOffset,
                                 accessor.count * dataType->Stride)};


        }

        void LoadResources()
        {
            std::ranges::transform(m_document.textures, std::back_inserter(m_textures),
                                   std::bind_front(&Loader::LoadTexture, this));

            std::ranges::transform(m_document.meshes, std::back_inserter(m_meshes), std::bind_front(&Loader::LoadMesh, this));
        }

        std::shared_ptr<ITexture> LoadTexture(const fx::gltf::Texture& texture)
        {
            auto loadedTexture = [&]() -> std::shared_ptr<ITexture> {
                if (texture.source < 0)
                    return nullptr;

                const auto& image = m_document.images[texture.source];
                if (image.bufferView >= 0 && image.uri.empty())
                {
                    const auto& bufferView = m_document.bufferViews[image.bufferView];
                    if (bufferView.buffer < 0)
                        throw std::logic_error("invalid buffer_view");

                    const auto& buffer = m_document.buffers[bufferView.buffer];
                    return TextureLoader::LoadTexture(
                        m_renderer,
                        std::as_bytes(std::span {buffer.data}).subspan(bufferView.byteOffset, bufferView.byteLength));
                }

                if (image.IsEmbeddedResource())
                {
                    std::vector<uint8_t> embeddedData;
                    image.MaterializeData(embeddedData);
                    return TextureLoader::LoadTexture(m_renderer, std::as_bytes(std::span {embeddedData}));
                }

                auto fullPath = m_currentPath / image.uri;
                return TextureLoader::LoadTexture(m_renderer, fullPath.generic_string());
            }();

            if (!loadedTexture || texture.sampler < 0)
                return loadedTexture;

            const auto& sampler = m_document.samplers[texture.sampler];

            if (sampler.wrapS != sampler.wrapT)
                Log::Warning() << "wrapS != wrapT, using first" << std::endl;

            loadedTexture->SetWrapMode(TranslateWrappingMode(sampler.wrapS));
            return loadedTexture;

            //TODO: support all sampler parameters
        }


        std::unique_ptr<IUniformContainer> TranslateMaterial(const fx::gltf::Material& material)
        {
            auto container = std::make_unique<UniformContainer>();

            auto trySetTexture = [&](const std::string& paramName, const fx::gltf::Material::Texture& texture, std::shared_ptr<ITexture> defaultValue = nullptr) {
                if (!texture.empty())
                    container->SetUniform(paramName, m_textures[texture.index]);
                else if (defaultValue)
                    container->SetUniform(paramName, defaultValue);
            };

            trySetTexture("u_texAlbedo"s, material.pbrMetallicRoughness.baseColorTexture);
            trySetTexture("u_texAoRoughnessMetallic"s, material.pbrMetallicRoughness.metallicRoughnessTexture);
            trySetTexture("u_texNormalMap"s, material.normalTexture, m_normalMapPlaceholder);

            return container;
        }

        SubmeshGroup LoadMesh(const fx::gltf::Mesh& gltfMesh)
        {
            const static std::pair<uint32_t, std::string> requiredAttributes[] = {
                 {1u, "POSITION"s},  {2u, "TEXCOORD_0"s},  {3u, "NORMAL"s}
            }; //"TANGENT"



            SubmeshGroup result {gltfMesh.primitives.size()};
            for (size_t index = 0; const auto& primitive : gltfMesh.primitives)
            {
                const auto& rf = m_renderer->GetResourceFactory();
                auto vao = rf.CreateVertexArray();
                auto vb = rf.CreateVertexBuffer(VertexBufferType::ArrayBuffer);
                

                std::vector <std::pair<uint32_t, BufferDataInfo>> buffersData;
                for (const auto& [attribIndex, attribName] : requiredAttributes)
                {
                    if (auto it = primitive.attributes.find(attribName); it != primitive.attributes.end())
                        buffersData.emplace_back(attribIndex, GetData(it->second));
                }

                const auto totalSize = std::accumulate(buffersData.begin(), buffersData.end(), size_t {},
                    [](size_t size, const auto& bdi) { return size + bdi.second.data.size_bytes(); });

                vb->SetDataRaw(std::span<const std::byte> {static_cast<const std::byte*>(nullptr), totalSize});

                for (size_t startPos = 0; auto& [attribIndex, bufferData] : buffersData)
                {
                    const size_t currentDataLength = bufferData.data.size_bytes();
                    {
                        BufferMapperGuard guard {*vb, startPos, currentDataLength, BufferUsage::Write};
                        std::memcpy(guard.data(), bufferData.data.data(), currentDataLength);
                    }
                    bufferData.bindingParams.Offset = startPos;

                    vao->SetAttributeBinding(attribIndex, vb, bufferData.bindingParams);
                    startPos += currentDataLength;
                }

                if (primitive.indices < 0)
                    throw std::logic_error("index buffer is required");

                const auto indexBufferInfo = GetData(primitive.indices);

                auto ib = rf.CreateVertexBuffer(VertexBufferType::IndexBuffer, indexBufferInfo.data);
                vao->SetIndexBuffer(ib, indexBufferInfo.bindingParams.Type);
     

                auto mesh = std::make_unique<Mesh>("Primitive submesh #"s + std::to_string(index));
                mesh->VertexArray = std::move(vao);
                mesh->SubMeshes.emplace_back(std::vector{MeshChunk {Primitives::Triangles {}, 0, indexBufferInfo.count, 0}});
                if (primitive.material >= 0)
                    mesh->Materials.emplace_back(TranslateMaterial(m_document.materials[primitive.material]));

                
                //GetAttributeData()

                result[index++] = std::move(mesh);
            }

            return result;
        }
    };
}

std::shared_ptr<Node> GltfMeshLoader::LoadScene(std::shared_ptr<IRenderer> renderer, const str& sv)
{
    Log::Info() << "Loading model from '" << sv << "'." << std::endl;

    Loader loader {std::move(renderer), sv};
    return loader.BuildScene();
}
