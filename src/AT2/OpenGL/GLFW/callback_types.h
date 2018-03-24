#ifndef CALLBACK_TYPES_HEADER
#define CALLBACK_TYPES_HEADER

#include <glm/vec2.hpp>

class MousePos
{
public:
	MousePos(const glm::vec2& pos, const glm::vec2& previousPos) : m_pos(pos), m_prevPos(previousPos) {}

	const glm::vec2& getPos() const			{ return m_pos; }
	glm::vec2 getDeltaPos() const			{ return m_pos - m_prevPos; }
	const glm::vec2& getPreviousPos() const	{ return m_prevPos; }

private:
	glm::vec2 m_pos, m_prevPos;
};


#endif