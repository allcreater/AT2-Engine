#include "MeshResource.h"
#include "MeshLoader.h"

using namespace AT2::Resources;

std::shared_ptr<void> MeshResource::Load(const std::shared_ptr<IResourceFactory>& resourceFactory)
{
    if (!m_source->IsReloadable())
        return nullptr;

    if (auto* fileSource = dynamic_cast<FileDataSource*>(m_source.get()))
        return MeshLoader::LoadNode(resourceFactory, fileSource->getFilename().string());
    
    throw AT2Exception {AT2Exception::ErrorCase::NotImplemented, "Mesh load from stream is not supported yet"}; //TODO: is it possible with assimp ?
}
