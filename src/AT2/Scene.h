#pragma once

#include "AT2.h"
#include "camera.h"

//TODO encapsulate all graphic API calls

namespace AT2
{
class Node;

using NodeRef = std::shared_ptr<Node>;

struct NodeVisitor
{
    //don't sure are we really need double dispatching so that we will use RTTI instead instead of using different versions of functions there
    virtual void Visit(Node& node) {} 
    virtual void UnVisit(Node& node) {}

    virtual ~NodeVisitor() = default;
};

class Node
{
public:
    Node() = default;
    Node(std::string name) : name(std::move(name)) {}

    virtual void Accept(NodeVisitor& nv)
    {
        nv.Visit(*this);

        for (auto& node : child_nodes)
            node->Accept(nv);

        nv.UnVisit(*this);
    }

    Node& AddChild(NodeRef node)
    {
        return *child_nodes.emplace_back(std::move(node));
    }

    template <typename T>
    T& GetChild(size_t index)
    {
        return dynamic_cast<T&>(*child_nodes[index]);
    }

    const glm::mat4& GetTransform() const noexcept { return transform; }
    void SetTransform(const glm::mat4& mat) noexcept { transform = mat; }

    void SetName(std::string newName) { name = std::move(newName); }
    const std::string& GetName() const noexcept { return name; }


    virtual ~Node() = default;

protected:
    std::string name;
    glm::mat4 transform {1.0f};
    std::vector<NodeRef> child_nodes;
};

struct SubMesh
{
    SubMesh() = default;
    SubMesh(SubMesh&&) = default;
    SubMesh& operator=(SubMesh&&) = default;

    SubMesh(const SubMesh&) = delete;
    SubMesh& operator=(const SubMesh&) = delete;

    //SubMesh(std::vector<std::unique_ptr<IDrawPrimitive>> primitives) : Primitives(std::move(primitives)) {}
    std::string Name;

    TextureSet Textures;
    std::shared_ptr<IUniformContainer> UniformBuffer;
    std::vector<std::unique_ptr<IDrawPrimitive>> Primitives;
};

struct Mesh
{
    Mesh() = default;
    Mesh(Mesh&&) = default;
    Mesh& operator=(Mesh&&) = default;

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    std::string Name;
    std::shared_ptr<IShaderProgram> Shader;
    std::shared_ptr<IVertexArray> VertexArray;
    std::shared_ptr<IUniformContainer> UniformBuffer;

    std::vector<SubMesh> Submeshes; //TODO: move Mesh and Submesh from scene so that nodes could be builded by Mesh
};

class MeshNode : public Node
{
public:
    void SetMesh(Mesh&& newMesh) { mesh = std::move(newMesh); }
    Mesh& GetMesh() noexcept { return mesh; }

private:
    Mesh mesh;
};

struct SphereLight {};

struct DirectionalLight
{
    glm::vec3 Direction;
};

struct LightNode : Node
{
    using LightFlavor = std::variant<SphereLight, DirectionalLight>;

    LightNode() = default;
    LightNode(LightFlavor flavor, glm::vec3 color) : flavor(flavor), intensity(color) { UpdateEffectiveRadius(); }

    LightNode& SetIntensity(glm::vec3 newIntensity)
    {
        intensity = newIntensity;
        UpdateEffectiveRadius();

        return *this;
    }
    const glm::vec3& GetIntensity() const noexcept { return intensity; }

    float GetEffectiveRadius() const noexcept { return m_effectiveRadius; }

    const LightFlavor& GetFlavor() const noexcept { return flavor; }

protected:
    void UpdateEffectiveRadius()
    {
        m_effectiveRadius = glm::length(intensity) * 0.5; //TODO: think about it :)
    }

private:
    LightFlavor flavor = SphereLight{};
    glm::vec3 intensity = { 100, 100, 100 };
    float m_effectiveRadius = 10.0;
};


class DrawableNode : public Node
{
public:
    size_t SubmeshIndex;
};

class Scene
{
public:
    Node& GetRoot() { return *root; }

private:
    NodeRef root = std::make_shared<Node>();
};

};
