#ifndef AT2_OPENGL_LOWLEVEL_HEADER
#define AT2_OPENGL_LOWLEVEL_HEADER

#include <gl/glew.h>

#include <sstream>
#include <vector>

#include "../AT2.h"


namespace AT2::OpenGL::Utils
{
    inline void SetGlState(GLenum state, bool enabled)
    {
        if (enabled)
            glEnable(state);
        else
            glDisable(state);
    }
}

#endif