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
        virtual bool Visit(Node& node) { return false; }
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

        void SetEnabled(bool enabled) { this->enabled = enabled; }

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
        void SetMesh(MeshRef newMesh) { mesh = std::move(newMesh); }
        [[nodiscard]] ConstMeshRef GetMesh() const noexcept { return mesh; }
        MeshRef GetMesh() noexcept { return mesh; }

    private:
        MeshRef mesh;
    };

    class DrawableNode : public Node
    {
    public:
        size_t SubmeshIndex = 0;
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
