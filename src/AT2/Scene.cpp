#include "Scene.h"


using namespace AT2;

//TODO: make it generic
struct NodeFindVisitor : NodeVisitor
{
    NodeFindVisitor(std::string_view name, const std::type_info* nodeType) :
        name(name), nodeType(nodeType) {}

    bool Visit(Node& node) override
    {
        if (node.GetName() == name && (!nodeType || *nodeType == typeid(node)))
        {
            found = &node;
            return false;
        }

        return true;
    }

    void UnVisit(Node& node) override
    {
    }

    Node* GetFoundNode() const noexcept { return found; }

private:
    std::string_view name;
    const std::type_info* nodeType;
    Node* found = nullptr;
};

Node* Scene::FindNode(std::string_view name, const std::type_info* nodeType) const
{
    NodeFindVisitor visitor{ std::move(name), nodeType};
    GetRoot().Accept(visitor);

    return visitor.GetFoundNode();
}
