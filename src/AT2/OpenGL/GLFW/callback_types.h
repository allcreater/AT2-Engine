#ifndef CALLBACK_TYPES_HEADER
#define CALLBACK_TYPES_HEADER

#include <glm/vec2.hpp>

class MousePos
{
public:
    MousePos() = default;
    MousePos(const glm::vec2& pos, const glm::vec2& previousPos) : position(pos), previous_position(previousPos) {}
    MousePos(const MousePos&) = default;
    MousePos& operator=(const MousePos&) = default;

    const glm::dvec2& getPos() const noexcept { return position; }
    glm::dvec2 getDeltaPos() const noexcept { return position - previous_position; }
    const glm::dvec2& getPreviousPos() const noexcept { return previous_position; }

private:
    glm::dvec2 position, previous_position;
};


#endif