#ifndef LINES_HELPER_HEADER
#define LINES_HELPER_HEADER

#include <Mesh.h>

class LinesHelper
{
public:
    LinesHelper() = default;

    void Draw(AT2::IRenderer& renderer);

    void AddLine(const glm::vec2& begin, const glm::vec2& end, const glm::vec4& color = glm::vec4(1.0, 1.0, 1.0, 1.0));
    void Clear();

private:
    void Init(const AT2::IRenderer& renderer);
    void UpdateVAO(const AT2::IRenderer& renderer);

private:
    std::shared_ptr<AT2::IVertexArray> m_VAO;
    std::vector<glm::vec2> m_vertices;
    std::vector<glm::vec4> m_colors;
    bool m_vertexBufferNeedUpdate = true;
};

#endif
