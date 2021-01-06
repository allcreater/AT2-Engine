#ifndef AT2_OPENGL_LOWLEVEL_HEADER
#define AT2_OPENGL_LOWLEVEL_HEADER

#include <glad/glad.h>

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

    inline int GetInteger(GLenum parameter, GLint min = std::numeric_limits<GLint>::min(),
                      GLint max = std::numeric_limits<GLint>::max())
    {
        GLint result = 0;
        glGetIntegerv(parameter, &result);

        if (result < min || result > max)
            throw AT2Exception(AT2Exception::ErrorCase::Renderer, "renderer capabilities query error");

        return result;
    }
}

#endif