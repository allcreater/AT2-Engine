#pragma once

#include <StateManager.h>

namespace AT2::Metal
{

class StateManager : public StateManager
{
    using StateManager::StateManager;

    void ApplyState(RenderState state) override;
};

};
