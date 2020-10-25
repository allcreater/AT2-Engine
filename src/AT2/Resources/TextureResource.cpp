#include "TextureResource.h"
#include "TextureLoader.h"

using namespace AT2::Resources;

std::shared_ptr<void> TextureResource::Load(const std::shared_ptr<AT2::IResourceFactory>& resourceFactory)
{
    if (!m_source->IsReloadable())
        return nullptr;

    if (auto* fileSource = dynamic_cast<FileDataSource*>(m_source.get()))
        return TextureLoader::LoadTexture(resourceFactory, fileSource->getFilename().string());
    else
    {
        auto data = m_source->LoadBytes();
        return TextureLoader::LoadTexture(resourceFactory, data.data(), data.size());
    }
}
