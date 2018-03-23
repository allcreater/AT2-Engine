#ifndef AT2_GL_TIMERQUERY_H
#define AT2_GL_TIMERQUERY_H

#include "AT2lowlevel.h"

namespace AT2
{

	class GlTimerQuery
	{
	public:
		GlTimerQuery();
		~GlTimerQuery();

		void Begin();
		void End();
		GLuint64 WaitForResult();

	private:
		GLuint m_id;
		GLuint64 m_resultValue;
	};

}
#endif