#pragma once

#include <StateManager.h>

namespace AT2::Metal
{

class MtlStateManager : public StateManager
{
    using StateManager::StateManager;

    void ApplyState(RenderState state) override;
};

};
