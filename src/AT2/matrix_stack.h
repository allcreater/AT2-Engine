#pragma once

#include <stack>
#include <vector>
#include <glm/glm.hpp>

namespace AT2
{

class MatrixStack
{
public:
    [[nodiscard]] const glm::mat4& getModelView() const noexcept { return mat_model_view; }
    [[nodiscard]] const glm::mat4& getProjection() const noexcept { return mat_projection; }
    [[nodiscard]] const glm::mat4& getModelViewInverse() const noexcept { return mat_model_view_inverse; }
    [[nodiscard]] const glm::mat4& getProjectionInverse() const noexcept { return mat_projection_inverse; }

    [[nodiscard]] bool empty() const noexcept { return model_view_matrices.empty() && projection_matrices.empty(); }

    void reset(glm::mat4 initialModelView, glm::mat4 initialProjection)
    {
        mat_model_view = initialModelView;
        mat_projection = initialProjection;
        mat_model_view_inverse = glm::inverse(mat_model_view);
        mat_projection_inverse = glm::inverse(mat_projection);


        //I hope it's right way to perform clearing :)
        decltype(model_view_matrices) emptyMV{}, emptyProj{};
        model_view_matrices.swap(emptyMV);
        projection_matrices.swap(emptyProj);

        assert(model_view_matrices.empty());
        assert(projection_matrices.empty());
    }

    void pushModelView(const glm::mat4& matrix, bool multiply = true)
    {
        model_view_matrices.push(mat_model_view);
        if (multiply)
            mat_model_view *= matrix;
        else
            mat_model_view = matrix;

        //probably could be optimized
        mat_model_view_inverse = glm::inverse(mat_model_view);
    }

    void popModelView()
    {
        if (model_view_matrices.empty())
            return;

        mat_model_view = model_view_matrices.top();
        model_view_matrices.pop();

        //probably could be optimized
        mat_model_view_inverse = glm::inverse(mat_model_view);
    }

    void pushProjection(const glm::mat4& matrix)
    {
        projection_matrices.push(mat_projection);
        mat_projection = matrix;

        //probably could be optimized
        mat_projection = glm::inverse(mat_projection);
    }

    void popProjection()
    {
        if (projection_matrices.empty())
            return;

        mat_projection = projection_matrices.top();
        projection_matrices.pop();

        //probably could be optimized
        mat_projection = glm::inverse(mat_projection);
    }


private:
    glm::mat4 mat_model_view{ 1.0f }, mat_projection{ 1.0f };
    glm::mat4 mat_model_view_inverse{ 1.0f }, mat_projection_inverse{ 1.0f };

    std::stack<glm::mat4, std::vector<glm::mat4>> model_view_matrices, projection_matrices;
};

}