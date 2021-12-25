#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

namespace AT2
{

    //TODO: integrate with Camera
    class Transform
    {
    public:
        Transform() { recalculate(); }
        Transform(glm::mat4 newTransformation)
        {
            fromMatrix(newTransformation);
        }

        void fromMatrix(glm::mat4 newTransformation)
        {
            glm::vec3 skew {};
            glm::vec4 perspective {};

            transformation = newTransformation;
            decompose(transformation, scale, rotation, position, skew, perspective);
        }



        [[nodiscard]] const glm::vec3& getPosition() const noexcept { return position; }
        [[nodiscard]] const glm::quat& getRotation() const noexcept { return rotation; }
        [[nodiscard]] const glm::vec3& getScale() const noexcept { return scale; }

        [[nodiscard]] const glm::mat4& asMatrix() const noexcept { return transformation; }
        [[nodiscard]] operator glm::mat4() const noexcept { return transformation; }

        Transform& setPosition(glm::vec3 newPosition)
        {
            position = newPosition;
            recalculate();

            return *this;
        }

        Transform& setRotation(glm::quat newRotation)
        {
            rotation = newRotation;
            recalculate();

            return *this;
        }

        Transform& setScale(glm::vec3 newScale)
        {
            scale = newScale;
            recalculate();

            return *this;
        }


    private:
        void recalculate()
        {
            transformation = glm::scale(translate(glm::mat4 {1.0f}, position) * glm::mat4_cast(rotation), scale);
        }

    private:
        glm::vec3 position {0.0f, 0.0f, 0.0f};
        glm::vec3 scale {1.0f, 1.0f, 1.0f};
        glm::quat rotation {1.0f, 0.0f, 0.0f, 0.0f};

        glm::mat4 transformation {};
    };

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