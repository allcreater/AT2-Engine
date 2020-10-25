#pragma once

#include "../Scene.h"

namespace AT2::Resources
{
    class MeshLoader
    {
    public:
        static std::unique_ptr<MeshNode> LoadNode(const std::shared_ptr<IResourceFactory>& resourceFactory,
                                                  const str& sv);
    };
} // namespace AT2
