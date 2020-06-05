#include "GlTimerQuery.h"

using namespace AT2;

GlTimerQuery::GlTimerQuery()
{
	glGenQueries(1, &m_id);
}
GlTimerQuery::~GlTimerQuery()
{
	glDeleteQueries(1, &m_id);
}

void GlTimerQuery::Begin()
{
	m_resultValue = 0;
	glBeginQuery(GL_TIME_ELAPSED, m_id);
}

void GlTimerQuery::End()
{
	glEndQuery(GL_TIME_ELAPSED);
}

GLuint64 GlTimerQuery::WaitForResult()
{
	m_resultValue = 0;
	glGetQueryObjectui64v(m_id, GL_QUERY_RESULT, &m_resultValue);
	return m_resultValue;
}