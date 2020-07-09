#ifndef AT2_GL_TIMERQUERY_H
#define AT2_GL_TIMERQUERY_H

#include "AT2lowlevel.h"

namespace AT2
{

    class GlTimerQuery
    {
    public:
        NON_COPYABLE_OR_MOVABLE(GlTimerQuery)

        GlTimerQuery();
        ~GlTimerQuery();

        void Begin();
        void End();
        GLuint64 WaitForResult();

    private:
        GLuint m_id {0};
        GLuint64 m_resultValue {0};
    };

} // namespace AT2
#endif