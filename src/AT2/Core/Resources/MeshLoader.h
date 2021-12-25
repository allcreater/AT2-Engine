#pragma once

#include <Scene/Scene.h>

namespace AT2::Resources
{
    class MeshLoader
    {
    public:
        static std::shared_ptr<Scene::Node> LoadNode(std::shared_ptr<IRenderer> renderer, const str& sv);
    };
} // namespace AT2
