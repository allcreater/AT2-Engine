#include "GltfSceneLoader.h"

#include <bit>
#include <ranges>
#include <fx/gltf.h>
#include <glm/packing.hpp>
#include <filesystem>

#include "../Animation.h"
#include "../BufferMapperGuard.h"
#include "TextureLoader.h"

using namespace AT2;
using namespace AT2::Scene;
using namespace AT2::Resources;
using namespace std::literals;

namespace
{
    //from gltf viewer example
    std::optional<BufferBindingParams> TranslateDataType(fx::gltf::Accessor const& accessor) noexcept
    {
        BufferBindingParams result;
        result.Count = 1;

        using ComponentType = fx::gltf::Accessor::ComponentType;
        switch (accessor.componentType)
        {
        case ComponentType::Byte:
            result.Type = BufferDataType::Byte;
            result.Stride = 1;
            break;
        case ComponentType::UnsignedByte:
            result.Type = BufferDataType ::UByte;
            result.Stride = 1;
            break;
        case ComponentType::Short:
            result.Type = BufferDataType::Short;
            result.Stride = 2;
            break;
        case ComponentType::UnsignedShort:
            result.Type = BufferDataType ::UShort;
            result.Stride = 2;
            break;
        case ComponentType::Float:
            result.Type = BufferDataType ::Float;
            result.Stride = 4;
            break;
        case ComponentType::UnsignedInt:
            result.Type = BufferDataType ::UInt;
            result.Stride = 4;
            break;
        case ComponentType::None: throw std::logic_error("unsupported data type");
        }

        using Type = fx::gltf::Accessor::Type;
        switch (accessor.type)
        {
        case Type::Mat2: result.Stride *= 4; break;
        case Type::Mat3: result.Stride *= 9; break;
        case Type::Mat4: result.Stride *= 16; break;
        case Type::Scalar: break;
        case Type::Vec2:
            result.Count = 2;
            result.Stride *= 2;
            break;
        case Type::Vec3:
            result.Count = 3;
            result.Stride *= 3;
            break;
        case Type::Vec4:
            result.Count = 4;
            result.Stride *= 4;
            break;
        case Type::None: throw std::logic_error("unsupported data type");
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

        assert(false);
    }

    constexpr std::optional<TextureMinificationMode> TranslateMinFilter(fx::gltf::Sampler::MinFilter minFilter)
    {
        using MinFilter = fx::gltf::Sampler::MinFilter;
        switch (minFilter)
        {
        case MinFilter::None:                   return {};
        case MinFilter::Nearest:                return TextureMinificationMode{TextureSamplingMode::Nearest, MimpapSamplingMode::Manual};
        case MinFilter::NearestMipMapNearest:   return TextureMinificationMode{TextureSamplingMode::Nearest, MimpapSamplingMode::Nearest};
        case MinFilter::NearestMipMapLinear:    return TextureMinificationMode{TextureSamplingMode::Nearest, MimpapSamplingMode::Linear};
        case MinFilter::Linear:                 return TextureMinificationMode{TextureSamplingMode::Linear, MimpapSamplingMode::Manual};
        case MinFilter::LinearMipMapNearest:    return TextureMinificationMode{TextureSamplingMode::Linear, MimpapSamplingMode::Nearest};
        case MinFilter::LinearMipMapLinear:     return TextureMinificationMode{TextureSamplingMode::Linear, MimpapSamplingMode::Linear};
        }

        assert(false);
    }

    constexpr std::optional<TextureSamplingMode> TranslateMagFilter(fx::gltf::Sampler::MagFilter magFilter)
    {
        using MagFilter = fx::gltf::Sampler::MagFilter;
        switch (magFilter)
        {
        case MagFilter::None: return {};
        case MagFilter::Nearest: return TextureSamplingMode::Nearest;
        case MagFilter::Linear: return TextureSamplingMode::Linear;
        }

        assert(false);
    }

    constexpr Animation::InterpolationMode TranslateInterpolationMode(fx::gltf::Animation::Sampler::Type interpolation)
    {
        using Type = fx::gltf::Animation::Sampler::Type;
        switch (interpolation)
        {
        case Type::Step: return Animation::Step{};
        case Type::Linear: return Animation::Linear{};
        case Type::CubicSpline: return Animation::CubicSpline{};
        }

        throw std::logic_error("invalid conversion");
    }


    class PlaceholderTextureCash
    {
        std::shared_ptr<IRenderer> m_renderer;
        std::unordered_map<glm::u32, std::shared_ptr<ITexture>> m_textures;

    public:
        explicit PlaceholderTextureCash(std::shared_ptr<IRenderer> renderer) : m_renderer(std::move(renderer)) {}

        //TODO: move to resource managing system and generalize for different texture formats
        std::shared_ptr<ITexture> GetTextureRGBA8(const glm::vec4& color)
        {
            const auto packedColor = glm::packUnorm4x8(color);

            if (auto it = m_textures.find(packedColor); it != m_textures.end())
                return it->second;

            auto texture = m_renderer->GetResourceFactory().CreateTexture(Texture2D {{1, 1}}, AT2::TextureFormats::RGBA8);
            texture->SubImage2D({}, {1, 1}, 0, TextureFormats::RGBA8, &packedColor);

            m_textures.emplace(packedColor, texture);

            return texture;
        }
    };

    class Loader
    {
        std::shared_ptr<IRenderer> m_renderer;
        fx::gltf::Document m_document;

        using SubmeshGroup = std::vector<MeshRef>;
        std::vector<SubmeshGroup> m_meshes;
        std::vector<std::shared_ptr<ITexture>> m_textures;
        std::vector<std::shared_ptr<Node>> m_nodes;
        std::vector<MeshComponent::SkeletonInstanceRef> m_skeletonInstances;
        std::filesystem::path m_currentPath;

        PlaceholderTextureCash m_placeholderTextureCash;

    public:
        Loader(std::shared_ptr<IRenderer> renderer, const str& sv)
        : m_renderer(std::move(renderer))
        , m_document(fx::gltf::LoadFromText(sv, fx::gltf::ReadQuotas {64, 64 * 1024 * 1024, 64 * 1024 * 1024}))
        , m_currentPath(sv)
        , m_nodes(m_document.nodes.size())
        , m_skeletonInstances (m_document.skins.size())
        , m_placeholderTextureCash(m_renderer)
        {
            m_currentPath.remove_filename();

            for (size_t skinIndex = 0; skinIndex < m_document.skins.size(); skinIndex++)
            {
                const auto& skin = m_document.skins[skinIndex];
                const auto inverseMatricesData = GetData(skin.inverseBindMatrices);

                m_skeletonInstances[skinIndex] =
                    std::make_shared<MeshComponent::SkeletonInstance>(Utils::reinterpret_span_cast<glm::mat4>(inverseMatricesData.data));
            }
        }

        NodeRef BuildScene()
        {
            Log::Info() << "Building scene graph..." << std::endl;

            LoadResources();

            NodeRef sceneRoot;

            if (m_document.scene >= 0 && static_cast<size_t>(m_document.scene) < m_document.scenes.size())
            {
                Log::Info() << "Building scene graph..." << std::endl;

                const auto& scene = m_document.scenes[m_document.scene];

                sceneRoot = std::make_shared<Node>(scene.name + " root"s);
                for (const auto nodeIndex : scene.nodes)
                {
                    BuildSceneGraph(nodeIndex, *sceneRoot);
                }
            }
            else
            {
                Log::Info() << "Scene graph is not available, loading as individual meshes" << std::endl;
                sceneRoot = std::make_shared<Node>("Model root"s);
                for (size_t meshIndex = 0; meshIndex < m_meshes.size(); ++meshIndex)
                {
                    auto childNode = std::make_shared<Node>();
                    sceneRoot->AddChild(childNode);
                    PopulateSubmeshes(*childNode, meshIndex);
                }
            }

            SetupAnimations();
            SetupSkins();

            return sceneRoot;
        }

    private:

        void SetupAnimations()
        {
            auto animationContainer = std::make_shared<AT2::Animation::AnimationCollection>();
            for (const auto& animation: m_document.animations)
            {
                auto& configuringAnimation = animationContainer->addAnimation(animation.name);

                for (const auto& channel : animation.channels)
                {
                    const auto& sampler = animation.samplers[channel.sampler];

                    const size_t animationNodeId = channel.target.node;
                    const auto animationComponent = m_nodes.at(animationNodeId)
                        ->getOrCreateComponent<Animation::AnimationComponent>(
                            animationContainer,
                            animationNodeId);
                    if (!animationComponent.isSameAs(animationContainer, animationNodeId))
                        throw std::logic_error("different animation components on one node");

                    const auto interpolationMode = TranslateInterpolationMode(sampler.interpolation);
                    auto inputChannelData = GetData(sampler.input);
                    auto outputChannelData = GetData(sampler.output);

                    assert(inputChannelData.bindingParams.Type == BufferDataType::Float &&
                           inputChannelData.bindingParams.Count == 1);

                    if (channel.target.path == "translation")
                    {
                        assert(outputChannelData.bindingParams.Type == BufferDataType::Float &&
                               outputChannelData.bindingParams.Count == 3);

                        configuringAnimation.addTrack(
                            animationNodeId,
                            Utils::reinterpret_span_cast<float>(inputChannelData.data),
                            Utils::reinterpret_span_cast<glm::vec3>(outputChannelData.data),
                            [](glm::vec3 value, Node& node)
                            {
                                node.GetTransform().setPosition(value); },
                            interpolationMode);

                    }
                    else if (channel.target.path == "rotation")
                    {
                        assert(outputChannelData.bindingParams.Type == BufferDataType::Float &&
                               outputChannelData.bindingParams.Count == 4);

                        configuringAnimation.addTrack(
                            animationNodeId,
                            Utils::reinterpret_span_cast<float>(inputChannelData.data),
                            Utils::reinterpret_span_cast<glm::quat>(outputChannelData.data),
                            [](glm::quat value, Node& node) {
                            node.GetTransform().setRotation(value); },
                            interpolationMode);

                    }
                    else if (channel.target.path == "scale")
                    {
                        assert(outputChannelData.bindingParams.Type == BufferDataType::Float &&
                               outputChannelData.bindingParams.Count == 3);

                        configuringAnimation.addTrack(
                            animationNodeId,
                            Utils::reinterpret_span_cast<float>(inputChannelData.data),
                            Utils::reinterpret_span_cast<glm::vec3>(outputChannelData.data),
                            [](glm::vec3 value, Node& node) {
                                node.GetTransform().setScale(value); },
                            interpolationMode);
                    }
                    //else if (channel.target.path == "weights")
                    
                }
            }
        }

        void SetupSkins()
        {
            for (size_t skinIndex = 0; skinIndex < m_document.skins.size(); skinIndex++)
            {
                const auto& skin = m_document.skins[skinIndex];

                for (size_t boneIndex = 0; auto boneNode : skin.joints)
                    m_nodes[boneNode]->getOrCreateComponent<Animation::BoneComponent>(boneIndex++, m_skeletonInstances[skinIndex]);
            }
        }

        // Scene graph

        void PopulateSubmeshes(Node& node, size_t meshIndex)
        {
            node.SetName("Mesh group '"s + m_document.meshes[meshIndex].name + "'"s);
            for (const auto& submesh : m_meshes[meshIndex])
                node.addComponent(std::make_unique<MeshComponent>(submesh, std::vector {0u}));
        }

        void BuildSceneGraph(int32_t nodeIndex, AT2::Scene::Node& baseNode)
        {
            if (nodeIndex < 0)
                throw std::logic_error("BuildSceneGraph: negative node index");

            const fx::gltf::Node& node = m_document.nodes[static_cast<size_t>(nodeIndex)];
            
            auto currentNode = std::make_shared<Node>(node.name);
            if (!std::ranges::equal(node.translation, fx::gltf::defaults::NullVec3) ||
                !std::ranges::equal(node.rotation , fx::gltf::defaults::IdentityRotation) ||
                !std::ranges::equal(node.scale , fx::gltf::defaults::IdentityVec3))
            {
                auto tr = translate(glm::mat4 {1.0}, std::bit_cast<glm::vec3>(node.translation))
                    * mat4_cast(std::bit_cast<glm::quat>(node.rotation));
                auto trs = scale(tr, std::bit_cast<glm::vec3>(node.scale));

                currentNode->SetTransform(trs);
            }
            else
                currentNode->SetTransform(std::bit_cast<glm::mat4>(node.matrix));

            baseNode.AddChild(currentNode);

            m_nodes[static_cast<size_t>(nodeIndex)] = currentNode;

            if (node.mesh >= 0)
                PopulateSubmeshes(*currentNode, static_cast<size_t>(node.mesh));
            if (node.skin >= 0)
            {
                Log::Debug() << "Skin at node [" << nodeIndex << "] '" << node.name << std::endl;
                for (auto* meshComponent : currentNode->getComponents<MeshComponent>())
                    meshComponent->setSkeletonInstance(m_skeletonInstances[node.skin]);
            }

            if (const size_t cameraIndex = node.camera; node.camera >= 0)
            {
            }

            for (const auto childIndex : node.children)
            {
                BuildSceneGraph(childIndex, *currentNode);
            }
        }

        // Materials and textures

        void LoadResources()
        {
            std::ranges::transform(m_document.textures, std::back_inserter(m_textures),
                                   std::bind_front(&Loader::LoadTexture, this));

            std::ranges::transform(m_document.meshes, std::back_inserter(m_meshes),
                                   std::bind_front(&Loader::LoadMesh, this));
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

            loadedTexture->SetWrapMode({TranslateWrappingMode(sampler.wrapS), TranslateWrappingMode(sampler.wrapT)});
            loadedTexture->SetSamplingMode(
                TextureSamplingParams {*TranslateMagFilter(sampler.magFilter), *TranslateMinFilter(sampler.minFilter)});

            return loadedTexture;

            //TODO: support all sampler parameters
        }


        std::unique_ptr<IUniformContainer> TranslateMaterial(const fx::gltf::Material& material)
        {
            auto container = std::make_unique<UniformContainer>();

            const auto trySetTexture = [&](const std::string& paramName, const fx::gltf::Material::Texture& texture, auto&& defaultValueGetter) {
                //Utils::lazy defaultValue {std::forward<decltyle(defaultValueGetter)>(defaultValueGetter)};
                if (!texture.empty())
                    container->SetUniform(paramName, m_textures[texture.index]);
                else
                    container->SetUniform(paramName, defaultValueGetter());
            };

            //functions to lazily getting default textures
            const auto defaultColor = [&] {
                
                return m_placeholderTextureCash.GetTextureRGBA8(std::bit_cast<glm::vec4>(material.pbrMetallicRoughness.baseColorFactor));
            };
            const auto defaultMetallicRoughness = [&] {
                return m_placeholderTextureCash.GetTextureRGBA8({1.0, material.pbrMetallicRoughness.roughnessFactor,
                                                                 material.pbrMetallicRoughness.metallicFactor, 1.0});
            };
            const auto defaultNormalMap = [this] {
                return m_placeholderTextureCash.GetTextureRGBA8({0.5, 0.5, 1.0, 1.0});
            };

            trySetTexture("u_texAlbedo"s, material.pbrMetallicRoughness.baseColorTexture, defaultColor);
            trySetTexture("u_texAoRoughnessMetallic"s, material.pbrMetallicRoughness.metallicRoughnessTexture, defaultMetallicRoughness);
            trySetTexture("u_texNormalMap"s, material.normalTexture, defaultNormalMap);

            return container;
        }

        // Geometry

        struct BufferDataInfo
        {
            BufferBindingParams bindingParams;
            uint32_t count;
            std::span<const std::byte> data;
        };

        //TODO: make type-safe!!!
        BufferDataInfo GetData(unsigned attribIndex)
        {
            //TODO: support sparce accesors

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

        SubmeshGroup LoadMesh(const fx::gltf::Mesh& gltfMesh)
        {
            const static std::pair<uint32_t, std::string> requiredAttributes[] = {
                {1u, "POSITION"s}, {2u, "TEXCOORD_0"s}, {3u, "NORMAL"s}, {4u, "JOINTS_0"}, {5u, "WEIGHTS_0"}}; //"TANGENT"


            SubmeshGroup result {gltfMesh.primitives.size()};
            for (size_t index = 0; const auto& primitive : gltfMesh.primitives)
            {
                const auto& rf = m_renderer->GetResourceFactory();
                auto vao = rf.CreateVertexArray();
                auto vb = rf.CreateVertexBuffer(VertexBufferType::ArrayBuffer);


                std::vector<std::pair<uint32_t, BufferDataInfo>> buffersData;
                for (const auto& [attribIndex, attribName] : requiredAttributes)
                {
                    if (auto it = primitive.attributes.find(attribName); it != primitive.attributes.end())
                        buffersData.emplace_back(attribIndex, GetData(it->second));
                }

                const auto totalSize =
                    std::accumulate(buffersData.begin(), buffersData.end(), size_t {},
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

                unsigned int primitivesCount = buffersData.front().second.count;
                if (primitive.indices >= 0)
                {
                    const auto indexBufferInfo = GetData(primitive.indices);

                    auto ib = rf.CreateVertexBuffer(VertexBufferType::IndexBuffer, indexBufferInfo.data);
                    vao->SetIndexBuffer(ib, indexBufferInfo.bindingParams.Type);

                    primitivesCount = indexBufferInfo.count;
                }

                auto mesh = std::make_unique<Mesh>("Primitive submesh #"s + std::to_string(index));
                mesh->VertexArray = std::move(vao);
                mesh->SubMeshes.emplace_back(std::vector {MeshChunk {Primitives::Triangles {}, 0, primitivesCount, 0}});
                if (primitive.material >= 0)
                    mesh->Materials.emplace_back(TranslateMaterial(m_document.materials[primitive.material]));


                result[index++] = std::move(mesh);
            }

            return result;
        }
    };
} // namespace

NodeRef GltfMeshLoader::LoadScene(std::shared_ptr<IRenderer> renderer, const str& sv)
{
    Log::Info() << "Loading model from '" << sv << "'." << std::endl;

    Loader loader {std::move(renderer), sv};
    return loader.BuildScene();
}
