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

class MeshNode : public Node
{
public:
    std::shared_ptr<IShaderProgram> Shader;
    std::shared_ptr<IVertexArray> VertexArray;
    std::shared_ptr<IUniformContainer> UniformBuffer;

private:

};

class DrawableNode : public Node
{
public:
    TextureSet Textures;
    std::shared_ptr<IUniformContainer> UniformBuffer;
    std::vector<std::unique_ptr<IDrawPrimitive>> Primitives;
};

class Scene
{
public:
    Node& GetRoot() { return *root; }

private:
    NodeRef root = std::make_shared<Node>();
};

};
