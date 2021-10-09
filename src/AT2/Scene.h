#pragma once

//#include <ranges>
#include <algorithm>
#include <chrono>

#include "AT2.h"
#include "Mesh.h"
#include "camera.h"
#include "matrix_stack.h"

//TODO: split into different headers

namespace AT2
{
    using Seconds = std::chrono::duration<double>;

    class ITime
    {
    public:
        [[nodiscard]] virtual Seconds getTime() const = 0;
        [[nodiscard]] virtual Seconds getDeltaTime() const = 0;
    };

}

namespace AT2::Scene
{
    class Node;

    using NodeRef = std::shared_ptr<Node>;

    struct NodeVisitor
    {
        //don't sure are we really need double dispatching so that we will use RTTI instead of using different versions of functions there
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

        Node* getParent() const noexcept { return m_parent; }

        void doUpdate(class UpdateVisitor&);

    protected:
        virtual void update(class UpdateVisitor&) = 0;

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
        NodeComponent& addComponent(std::unique_ptr<NodeComponent> component);

        template <typename T>
        requires(std::is_base_of_v<NodeComponent, T>) const std::vector<T*>& getComponents()
        {
            static std::vector<T*> s_result;
            s_result.resize(0);
            //return getComponentList() |
            //    std::ranges::views::transform([](std::unique_ptr<NodeComponent>& component) { return component.get(); }) |
            //    std::ranges::views::filter( [](NodeComponent* component) { return dynamic_cast<T*>(component); });
            std::transform(m_componentList.begin(), m_componentList.end(), std::back_inserter(s_result),
                           [](std::unique_ptr<NodeComponent>& component) { return dynamic_cast<T*>(component.get()); });
            s_result.erase(std::remove(s_result.begin(), s_result.end(), nullptr),
                s_result.end());

            return s_result;
        }

        template <typename T>
        requires(std::is_base_of_v<NodeComponent, T>) T* getComponent()
        {
            auto it = std::find_if(m_componentList.begin(), m_componentList.end(),
                                   [](const std::unique_ptr<NodeComponent>& component) {
                                       return dynamic_cast<T*>(component.get()) != nullptr;
                                   });
            return (it != m_componentList.end()) ? static_cast<T*>(it->get()) : nullptr;
        }

        template <typename T, typename ... Args>
        requires(std::is_base_of_v<NodeComponent, T>) T& getOrCreateComponent(Args ... args)
        {
            if (auto* existing = getComponent<T>())
                return *existing;

            return static_cast<T&>(addComponent(std::make_unique<T>(std::forward<Args>(args)...)));
        }

        template <typename T, typename... Args>
        requires(std::is_base_of_v<NodeComponent, T>) T& createUniqueComponent(Args... args)
        {
            if (auto* existing = getComponent<T>())
                throw std::logic_error("component should not be duplicated!");

            return static_cast<T&>(addComponent(std::make_unique<T>(std::forward<Args>(args)...)));
        }
    };

    class UpdateVisitor : public NodeVisitor
    {
        MatrixStack m_transforms;
        const ITime& m_timeSource;

    public:
        UpdateVisitor(const ITime& timeSource) : m_timeSource(timeSource) {}

        [[nodiscard]] const MatrixStack& getTransformsStack() const noexcept { return m_transforms; }
        [[nodiscard]] const ITime& getTime() const noexcept { return m_timeSource; }

        //TODO: some way to send messages down to hierarchy

    protected:
        bool Visit(Node& node) override;
        void UnVisit(Node& node) override;
    };

    //TODO: turn into components 
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

    class MeshComponent : public NodeComponent
    {
    public:
        class SkeletonInstance
        {
            std::vector<glm::mat4> m_inverseBindMatrices; //TODO: could be one for all instance
            std::vector<glm::mat4> m_resultJointTransforms;

        public:
            SkeletonInstance(std::span<const glm::mat4> inverseBindMatrices) :
                m_inverseBindMatrices(inverseBindMatrices.begin(), inverseBindMatrices.end()),
                m_resultJointTransforms(inverseBindMatrices.size())
            {
            }

            void calculateBoneTransform(size_t boneIndex, const glm::mat4& boneTransform)
            {
                if (boneIndex >= m_inverseBindMatrices.size())
                    throw std::out_of_range("bone index");

                m_resultJointTransforms[boneIndex] = boneTransform * m_inverseBindMatrices[boneIndex];
            }

            std::span<const glm::mat4> getResultJointTransforms() const noexcept { return m_resultJointTransforms; }
        };

        using SkeletonInstanceRef = std::shared_ptr<SkeletonInstance>;


        MeshComponent() = default;
        MeshComponent(MeshRef mesh, std::vector<unsigned> submeshIndices) :
            m_mesh(std::move(mesh)), m_submeshIndices(std::move(submeshIndices)) {}


        void setSkeletonInstance(SkeletonInstanceRef skeletonInstance) { m_skeletonInstance = std::move(skeletonInstance);}
        [[nodiscard]] const SkeletonInstanceRef& getSkeletonInstance() const { return m_skeletonInstance; }

        void setMesh(MeshRef newMesh) { m_mesh = std::move(newMesh); }
        [[nodiscard]] ConstMeshRef getMesh() const noexcept { return m_mesh; }
        [[nodiscard]] MeshRef getMesh() noexcept { return m_mesh; }

        std::span<const unsigned> GetSubmeshIndices() const noexcept { return m_submeshIndices; }

        void update(UpdateVisitor&) override {}

    private:
        MeshRef m_mesh;
        SkeletonInstanceRef m_skeletonInstance;

        std::vector<unsigned> m_submeshIndices;

    };


    class BoneComponent : public Scene::NodeComponent
    {
        size_t m_boneIndex;
        Scene::MeshComponent::SkeletonInstanceRef m_skeletonInstance;

    public:
        BoneComponent(size_t boneIndex, Scene::MeshComponent::SkeletonInstanceRef skeletonInstanceRef) :
            m_boneIndex(boneIndex), m_skeletonInstance(std::move(skeletonInstanceRef))
        {
        }

        void update(Scene::UpdateVisitor& updateVisitor) override
        {
            m_skeletonInstance->calculateBoneTransform(m_boneIndex, updateVisitor.getTransformsStack().getModelView());
        }
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
        void Update(const ITime& time);

    private:
        NodeRef root = std::make_shared<Node>();
    };

}; // namespace AT2
