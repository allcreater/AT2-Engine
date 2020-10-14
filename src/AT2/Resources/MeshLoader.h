#pragma once

#include "../Scene.h"

namespace AT2::Resources
{
    class MeshLoader
    {
    public:
        static std::unique_ptr<MeshNode> LoadNode(std::shared_ptr<IRenderer> renderer, const str& sv);
    };
} // namespace AT2
