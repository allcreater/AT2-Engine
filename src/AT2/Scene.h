#pragma once

#include "AT2.h"
#include "Mesh.h"
#include "camera.h"

//TODO encapsulate all graphic API calls

namespace AT2
{
    class Node;

    using NodeRef = std::shared_ptr<Node>;

    struct NodeVisitor
    {
        //don't sure are we really need double dispatching so that we will use RTTI instead instead of using different versions of functions there
        virtual bool Visit(Node& ) { return false; }
        virtual void UnVisit(Node& ) {}

        virtual ~NodeVisitor() = default;
    };

    template <typename Func>
    class FuncNodeVisitor : public NodeVisitor
    {
        Func visitFunc;
    public:
        FuncNodeVisitor(Func&& func) : visitFunc(std::forward<Func>(func)) {}

        bool Visit(Node& node) override { return visitFunc(node); }
    };


    //TODO: refactor?
    class NodeComponent
    {
        Node* m_parent = nullptr;
        friend class Node;

    public:
        virtual ~NodeComponent() = default;
        virtual void update(double t) = 0; //TODO: time should be available without params

        Node* getParent() const noexcept { return m_parent; }

    private:
        void setParent(Node& newParent) { m_parent = &newParent; }
    };

    class Node
    {
        using ComponentList = std::vector<std::unique_ptr<NodeComponent>>;

        std::string m_name;
        Transform m_transform;
        std::vector<NodeRef> child_nodes;
        ComponentList m_componentList;

    public:
        Node() = default;
        Node(std::string name) : m_name(std::move(name)) {}
        virtual ~Node() = default;

        virtual void Accept(NodeVisitor& nv)
        {
            if (nv.Visit(*this))
            {
                for (auto& node : child_nodes)
                    node->Accept(nv);
            }

            nv.UnVisit(*this);
        }

        Node& AddChild(NodeRef node) { return *child_nodes.emplace_back(std::move(node)); }

        template <typename T>
        T& GetChild(size_t index)
        {
            return dynamic_cast<T&>(*child_nodes[index]);
        }

        [[nodiscard]] const Transform& GetTransform() const noexcept { return m_transform; }
        [[nodiscard]] Transform& GetTransform() noexcept { return m_transform; }
        void SetTransform(const Transform& transform) noexcept { m_transform = transform; }

        [[nodiscard]] const std::string& GetName() const noexcept { return m_name; }
        void SetName(std::string newName) { m_name = std::move(newName); }

        [[nodiscard]] const ComponentList& getComponentList() const noexcept { return m_componentList; }
        NodeComponent& addComponent(std::unique_ptr<NodeComponent> component)
        {
            auto* pComponent = component.get();
            pComponent->setParent(*this);

            m_componentList.push_back(std::move(component));
            return *pComponent;
        }

        template <typename T>
        requires(std::is_base_of_v<NodeComponent, T>)
        T* getComponent()
        {
            auto it = std::find_if(m_componentList.begin(), m_componentList.end(),
                                   [](const std::unique_ptr<NodeComponent>& component) {
                                       return dynamic_cast<T*>(component.get()) != nullptr;
                                   });
            return (it != m_componentList.end()) ? static_cast<T*>(it->get()) : nullptr;
        }

        template <typename T, typename ... Args>
        requires(std::is_base_of_v<NodeComponent, T>)
        T& getOrCreateComponent(Args ... args)
        {
            if (auto* existing = getComponent<T>())
                return *existing;

            return static_cast<T&>(addComponent(std::make_unique<T>(std::forward<Args>(args)...)));
        }

    };

    struct SphereLight
    {
    };

    struct SkyLight
    {
        glm::vec3 Direction;
        std::shared_ptr<ITexture> EnvironmentMap;
    };

    struct LightNode : Node
    {
        using LightFlavor = std::variant<SphereLight, SkyLight>;

        LightNode() = default;
        LightNode(LightFlavor flavor, glm::vec3 color, std::string name) :
            Node(std::move(name)), flavor(std::move(flavor)), intensity(color)
        {
            UpdateEffectiveRadius();
        }

        LightNode& SetIntensity(glm::vec3 newIntensity)
        {
            intensity = newIntensity;
            UpdateEffectiveRadius();

            return *this;
        }

        void SetEnabled(bool _enabled) { enabled = _enabled; }

        [[nodiscard]] const glm::vec3& GetIntensity() const noexcept { return intensity; }
        [[nodiscard]] float GetEffectiveRadius() const noexcept { return effectiveRadius; }
        [[nodiscard]] const LightFlavor& GetFlavor() const noexcept { return flavor; }
        [[nodiscard]] bool GetEnabled() const noexcept { return enabled; }

    protected:
        void UpdateEffectiveRadius()
        {
            effectiveRadius = length(intensity) * 0.5f; //TODO: think about it :)
        }

    private:
        LightFlavor flavor = SphereLight {};
        glm::vec3 intensity = {100, 100, 100};
        float effectiveRadius = 10.0;
        bool enabled = true;
    };


    class MeshNode : public Node
    {
    public:
        MeshNode() = default;
        MeshNode(MeshRef mesh, std::vector<unsigned> submeshIndices, std::string name = {})
            : m_mesh(std::move(mesh))
            , m_submeshIndices(std::move(submeshIndices))
            , Node(std::move(name))
        {}

        void SetMesh(MeshRef newMesh) { m_mesh = std::move(newMesh); }
        [[nodiscard]] ConstMeshRef GetMesh() const noexcept { return m_mesh; }
        MeshRef GetMesh() noexcept { return m_mesh; }

        std::span<const unsigned> GetSubmeshIndices() const noexcept { return m_submeshIndices; }

    private:
        MeshRef m_mesh;
        std::vector<unsigned> m_submeshIndices;
    };

    class Scene
    {
    public:
        Node& GetRoot() const noexcept { return *root; }

        template <typename T, typename = std::enable_if_t<std::is_base_of_v<Node, T>>>
        T* FindNode(std::string_view name) const
        {
            return static_cast<T*>(FindNode(name, &typeid(T)));
        }

        Node* FindNode(std::string_view name, const std::type_info* nodeType = nullptr) const;

    private:
        NodeRef root = std::make_shared<Node>();
    };

}; // namespace AT2
