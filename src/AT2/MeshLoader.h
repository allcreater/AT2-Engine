#pragma once

#include "Scene.h"

namespace  AT2
{
    class MeshLoader
    {
    public:
        static NodeRef LoadNode(std::shared_ptr<IRenderer> renderer, const str& sv, const std::shared_ptr<IShaderProgram> &program);

    };
}
