#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

namespace AT2
{

    class Camera
    {
    public:
        Camera() { Recalculate(); }

        //world-space axises
        [[nodiscard]] glm::vec3 getLeft() const noexcept { return -view_inverse[0]; }
        [[nodiscard]] glm::vec3 getUp() const noexcept { return -view_inverse[1]; }
        [[nodiscard]] glm::vec3 getForward() const noexcept { return -view_inverse[2]; }

        //world-space orientation
        [[nodiscard]] const glm::vec3& getPosition() const noexcept { return position; }
        [[nodiscard]] const glm::quat& getRotation() const noexcept { return rotation; }

        [[nodiscard]] const glm::mat4& getView() const noexcept { return view; }
        [[nodiscard]] const glm::mat4& getViewInverse() const noexcept { return view_inverse; }
        [[nodiscard]] const glm::mat4& getProjection() const noexcept { return projection; }
        [[nodiscard]] const glm::mat4& getProjectionInverse() const noexcept { return projection_inverse; }

        Camera& setPosition(const glm::vec3 newPosition)
        {
            position = newPosition;
            Recalculate();

            return *this;
        }

        Camera& setRotation(const glm::quat& newRotation)
        {
            rotation = newRotation;
            Recalculate();

            return *this;
        }

        Camera& setView(const glm::mat4& newView)
        {
            glm::vec3 scale {}, skew {};
            glm::vec4 perspective {};
            glm::decompose(glm::inverse(newView), scale, rotation, position, skew, perspective);

            //right now could be optimized by manual view and view_inverse assigning
            Recalculate();

            return *this;
        }

        Camera& setProjection(const glm::mat4& newProjection)
        {
            projection = newProjection;
            projection_inverse = glm::inverse(projection);

            return *this;
        }

    private:
        void Recalculate()
        {
            //rotation = normalize(rotation);
            view_inverse = glm::translate(glm::mat4 {1.0f}, position) * glm::mat4_cast(rotation);
            view = glm::inverse(view_inverse);

            //glm::rotation()
        }

    private:
        //primary values
        glm::vec3 position {0, 0, 0};
        glm::quat rotation {1.0f, 0.0f, 0.0f, 0.0f};
        glm::mat4 projection {1.0};

        //derived values
        glm::mat4 view, view_inverse;
        glm::mat4 projection_inverse {1.0f};
    };

} // namespace AT2