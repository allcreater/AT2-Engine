#ifndef LINES_HELPER_HEADER
#define LINES_HELPER_HEADER
#include "../drawable.h"

class LinesHelper : public AT2::IDrawable
{
public:
	LinesHelper();

	void Draw(const std::shared_ptr<AT2::IRenderer>& renderer) override;

	void AddLine(const glm::vec2& begin, const glm::vec2& end, glm::vec4& color = glm::vec4(1.0, 1.0, 1.0, 1.0));
	void Clear();

private:
	void Init(const std::shared_ptr<AT2::IRenderer>& renderer);
	void UpdateVAO(const std::shared_ptr<AT2::IRenderer>& renderer);

private:
	std::shared_ptr<AT2::IVertexArray> m_VAO;
	std::vector<glm::vec2> m_vertices;
	std::vector<glm::vec4> m_colors;
	std::unique_ptr<AT2::IDrawPrimitive> m_DrawPrimitive;
	bool m_vertexBufferNeedUpdate = true;
};

#endif