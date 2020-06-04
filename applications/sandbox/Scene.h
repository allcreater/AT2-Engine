#pragma once

#include <map>
#include <AT2/AT2.h>
#include <AT2/camera.h>

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

    void AddChild(NodeRef node)
    {
        child_nodes.push_back(std::move(node));
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

    void Render(IRenderer& renderer, const Camera& camera);

private:
    NodeRef root = std::make_shared<Node>();
};

};
