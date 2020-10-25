#include "ResourceManager.h"

using namespace AT2::Resources;

void ResourceManager::AddResource(std::unique_ptr<IResource> resource, bool preload)
{
    const auto name = resource->GetName();
    auto [it, isInserted] = m_resources.try_emplace(name, Entry{std::move(resource)});
    if (!isInserted)
        throw ResourceManagerException("resource with that name already exists");

    if (preload)
    {
        it->second.GetOrLoad(LockResourceFactory());
    }
}

void ResourceManager::ReloadResources()
{
    const auto renderer = m_renderer.lock();
    if (!renderer)
        return;

    for (auto& [name, resource] : m_resources)
        resource.object = nullptr;
}

void ResourceManager::UnloadUnused()
{
}
