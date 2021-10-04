#include "Scene.h"


using namespace AT2;
using namespace AT2::Scene;

//TODO: make it generic
struct NodeFindVisitor : NodeVisitor
{
    NodeFindVisitor(std::string_view name, const std::type_info* nodeType) : name(name), nodeType(nodeType) {}

    bool Visit(Node& node) override
    {
        if (node.GetName() == name && (!nodeType || *nodeType == typeid(node)))
        {
            found = &node;
            return false;
        }

        return true;
    }

    void UnVisit(Node& node) override {}

    Node* GetFoundNode() const noexcept { return found; }

private:
    std::string_view name;
    const std::type_info* nodeType;
    Node* found = nullptr;
};

void NodeComponent::doUpdate( UpdateVisitor& updateVisitor)
{
    if (!getParent())
        return;

    update(updateVisitor);
}

NodeComponent& Node::addComponent(std::unique_ptr<NodeComponent> component)
{
    auto* pComponent = component.get();
    pComponent->setParent(*this);

    m_componentList.push_back(std::move(component));
    return *pComponent;
}

bool UpdateVisitor::Visit(Node& node)
{
    m_transforms.pushModelView(node.GetTransform().asMatrix());

    for (auto& component : node.getComponentList())
        component->doUpdate(*this);

    return true;
}

void UpdateVisitor::UnVisit(Node& node)
{
    m_transforms.popModelView();
}

Node* AT2::Scene::Scene::FindNode(std::string_view name, const std::type_info* nodeType) const
{
    NodeFindVisitor visitor {std::move(name), nodeType};
    GetRoot().Accept(visitor);

    return visitor.GetFoundNode();
}

void AT2::Scene::Scene::Update(const ITime& time)
{
    UpdateVisitor updateVisitor {time};
    GetRoot().Accept(updateVisitor);
}
