#pragma once

#include "AT2.h"
#include "camera.h"
#include "Mesh.h"

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

    [[nodiscard]] const glm::mat4& GetTransform() const noexcept { return transform; }
    void SetTransform(const glm::mat4& mat) noexcept { transform = mat; }

    void SetName(std::string newName) { name = std::move(newName); }
    [[nodiscard]] const std::string& GetName() const noexcept { return name; }


    virtual ~Node() = default;

protected:
    std::string name;
    glm::mat4 transform {1.0f};
    std::vector<NodeRef> child_nodes;
};

struct SphereLight {};

struct SkyLight
{
    glm::vec3 Direction;
    std::shared_ptr<ITexture> EnvironmentMap;
};

struct LightNode : Node
{
    using LightFlavor = std::variant<SphereLight, SkyLight>;

    LightNode() = default;
    LightNode(LightFlavor flavor, glm::vec3 color) : flavor(flavor), intensity(color) { UpdateEffectiveRadius(); }

    LightNode& SetIntensity(glm::vec3 newIntensity)
    {
        intensity = newIntensity;
        UpdateEffectiveRadius();

        return *this;
    }

    [[nodiscard]] const glm::vec3& GetIntensity() const noexcept { return intensity; }

    [[nodiscard]] float GetEffectiveRadius() const noexcept { return m_effectiveRadius; }

    [[nodiscard]] const LightFlavor& GetFlavor() const noexcept { return flavor; }

protected:
    void UpdateEffectiveRadius()
    {
        m_effectiveRadius = length(intensity) * 0.5f; //TODO: think about it :)
    }

private:
    LightFlavor flavor = SphereLight{};
    glm::vec3 intensity = { 100, 100, 100 };
    float m_effectiveRadius = 10.0;
};


class MeshNode : public Node
{
public:
    void SetMesh(Mesh&& newMesh) { mesh = std::move(newMesh); }
    [[nodiscard]] const Mesh& GetMesh() const noexcept { return mesh; }
    Mesh& GetMesh() noexcept { return mesh; }

private:
    Mesh mesh;
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
