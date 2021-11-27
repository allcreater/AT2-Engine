#ifndef CALLBACK_TYPES_HEADER
#define CALLBACK_TYPES_HEADER

#include <glm/vec2.hpp>

class MousePos
{
public:
    constexpr MousePos() = default;
    constexpr MousePos(const glm::vec2& pos, const glm::vec2& previousPos) :
        position(pos), previous_position(previousPos)
    {
    }
    constexpr MousePos(const MousePos&) = default;
    constexpr MousePos(MousePos&&) = default;
    constexpr MousePos& operator=(const MousePos&) = default;
    constexpr MousePos& operator=(MousePos&&) = default;
    ~MousePos() = default;

    [[nodiscard]] constexpr const glm::dvec2& getPos() const noexcept { return position; }
    [[nodiscard]] constexpr glm::dvec2 getDeltaPos() const noexcept { return position - previous_position; }
    [[nodiscard]] constexpr const glm::dvec2& getPreviousPos() const noexcept { return previous_position; }

private:
    glm::dvec2 position = {};
    glm::dvec2 previous_position = {};
};


#endif