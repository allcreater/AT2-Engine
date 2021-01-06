#pragma once

#include "../Scene.h"

namespace AT2::Resources
{
    class GltfMeshLoader
    {
    public:
        static std::shared_ptr<Node> LoadScene(std::shared_ptr<IRenderer> renderer, const str& sv);
    };
} // namespace AT2
