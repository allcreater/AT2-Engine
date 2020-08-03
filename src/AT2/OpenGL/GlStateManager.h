#ifndef GL_STATE_MANAGER_H
#define GL_STATE_MANAGER_H

#include "../StateManager.h"

namespace AT2
{

class GlStateManager : public StateManager
{
    using StateManager::StateManager;

    void ApplyState(RenderState state) override;
};

};

#endif