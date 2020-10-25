#pragma once

#include "../AT2.h"

#include <typeindex>
#include <typeinfo>
#include <utility>

namespace AT2::Resources
{
    class IResource
    {
    public:
        virtual ~IResource() = default;

    public:
        [[nodiscard]] virtual std::string_view GetName() const noexcept = 0;
        virtual std::shared_ptr<void> Load(const std::shared_ptr<IResourceFactory>& resourceFactory) = 0;
    };


    class ResourceManagerException : public AT2::AT2Exception
    {
        using AT2Exception::AT2Exception;
    };


    class ResourceManager final
    {
    public:

        explicit ResourceManager(std::weak_ptr<IRenderer> renderer) : m_renderer(std::move(renderer)) {}

        //template <typename T, typename... Args>
        //void MakeResource(Args&& ... args)
        //{
        //    AddResource(std::make_unique<T>(std::forward<Args>(args)...), false);
        //}

        [[nodiscard]] std::shared_ptr<IRenderer> GetRenderer() const noexcept
        {
            return m_renderer.lock();
        }


        void AddResource(std::unique_ptr<IResource> resource, bool preload = false);

        template<typename T>
        std::shared_ptr<T> Get(std::string_view name) noexcept
        {
            if (auto it = m_resources.find(name); it != m_resources.end())
                return std::static_pointer_cast<T>(it->second.GetOrLoad(LockResourceFactory())); //TODO:

            return nullptr;
        }

        //Should be asynchronous :/
        void ReloadResources();

        void UnloadUnused();

    private:
        std::weak_ptr<IRenderer> m_renderer;

        [[nodiscard]] std::shared_ptr<IResourceFactory> LockResourceFactory() const
        {
            const auto renderer = m_renderer.lock();
            return std::shared_ptr<IResourceFactory> {renderer, &renderer->GetResourceFactory()};
        }

        struct Entry
        {
            std::shared_ptr<void> GetOrLoad(const std::shared_ptr<IResourceFactory>& resourceFactory)
            {
                if (!object)
                    object = resource->Load(resourceFactory);
                
                return object;
            }

            std::unique_ptr<IResource> resource;
            std::shared_ptr<void> object = nullptr;
        };

        // Simplest possible data structure but in future could be replaced by something more smart
        std::unordered_map<std::string_view, Entry> m_resources;
    };

} // namespace AT2::Resources