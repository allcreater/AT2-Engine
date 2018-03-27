#include "drawable.h"

#include <AT2/OpenGl/GlDrawPrimitive.h> //Unfortunately right now it don't be created abstractrly

std::shared_ptr<AT2::MeshDrawable> AT2::MeshDrawable::MakeSphereDrawable(const std::shared_ptr<IRenderer>& renderer, int segX, int segY)
{
	std::vector<glm::vec3> normals; normals.reserve(segX * segY);
	std::vector<glm::uint> indices; indices.reserve(segX * segY * 6);

	for (int j = 0; j < segY; ++j)
	{
		double angV = j * pi / (segY - 1);
		for (int i = 0; i < segX; ++i)
		{
			double angH = i * pi * 2 / segX;

			normals.push_back(glm::vec3(sin(angV)*cos(angH), sin(angV)*sin(angH), cos(angV)));
		}
	}

	for (int j = 0; j < segY - 1; ++j)
	{
		const int nj = j + 1;
		for (int i = 0; i < segX; ++i)
		{
			const int ni = (i + 1) % segX;

			indices.push_back(j * segX + i);
			indices.push_back(j * segX + ni);
			indices.push_back(nj * segX + ni);
			indices.push_back(j * segX + i);
			indices.push_back(nj * segX + ni);
			indices.push_back(nj * segX + i);
		}
	}

	auto& rf = renderer->GetResourceFactory();
	
	auto vao = rf.CreateVertexArray();
	vao->SetVertexBuffer(1, rf.CreateVertexBuffer(AT2vbt::ArrayBuffer, AT2::BufferDataTypes::Vec3, normals.size() * sizeof(glm::vec3), normals.data()));
	vao->SetIndexBuffer(rf.CreateVertexBuffer(AT2vbt::IndexBuffer, AT2::BufferDataTypes::UInt, indices.size() * sizeof(glm::uint), indices.data()));

	auto drawable = std::make_shared<AT2::MeshDrawable>();
	drawable->Primitives.push_back(new AT2::GlDrawElementsPrimitive(AT2::GlDrawPrimitiveType::Triangles, indices.size(), AT2::GlDrawElementsPrimitive::IndicesType::UnsignedInt, 0));
	drawable->VertexArray = vao;

	return drawable;
}

std::shared_ptr<AT2::MeshDrawable> AT2::MeshDrawable::MakeTerrainDrawable(const std::shared_ptr<IRenderer>& renderer, int segX, int segY)
{
	std::vector<glm::vec2> texCoords(segX * segY * 4);//TODO! GlVertexBuffer - take iterators!

	for (int j = 0; j < segY; ++j)
	{
		for (int i = 0; i < segX; ++i)
		{
			const int num = (i + j * segX) * 4;
			texCoords[num] = glm::vec2(float(i) / segX, float(j) / segY);
			texCoords[num + 1] = glm::vec2(float(i + 1) / segX, float(j) / segY);
			texCoords[num + 2] = glm::vec2(float(i + 1) / segX, float(j + 1) / segY);
			texCoords[num + 3] = glm::vec2(float(i) / segX, float(j + 1) / segY);
		}
	}

	auto& rf = renderer->GetResourceFactory();

	auto vao = rf.CreateVertexArray();
	vao->SetVertexBuffer(1, rf.CreateVertexBuffer(AT2vbt::ArrayBuffer, AT2::BufferDataTypes::Vec2, texCoords.size()*sizeof(glm::vec2), texCoords.data()));


	auto drawable = std::make_shared<AT2::MeshDrawable>();
	drawable->Primitives.push_back(new AT2::GlDrawArraysPrimitive(AT2::GlDrawPrimitiveType::Patches, 0, texCoords.size()));
	drawable->VertexArray = vao;

	return drawable;
}

std::shared_ptr<AT2::MeshDrawable> AT2::MeshDrawable::MakeFullscreenQuadDrawable(const std::shared_ptr<IRenderer>& renderer)
{
	glm::vec3 positions[] = { glm::vec3(-1.0, -1.0, -1.0), glm::vec3(1.0, -1.0, -1.0), glm::vec3(1.0, 1.0, -1.0), glm::vec3(-1.0, 1.0, -1.0) };
	glm::uint indices[] = { 0, 1, 2, 0, 2, 3 };

	auto& rf = renderer->GetResourceFactory();

	auto vao = rf.CreateVertexArray();
	vao->SetVertexBuffer(1, rf.CreateVertexBuffer(AT2vbt::ArrayBuffer, AT2::BufferDataTypes::Vec3, 4*sizeof(glm::vec3), positions));
	vao->SetIndexBuffer(rf.CreateVertexBuffer(AT2vbt::IndexBuffer, AT2::BufferDataTypes::UInt, 6*sizeof(glm::uint), indices));

	auto drawable = std::make_shared<AT2::MeshDrawable>();
	drawable->Primitives.push_back(new AT2::GlDrawElementsPrimitive(AT2::GlDrawPrimitiveType::Triangles, 6, AT2::GlDrawElementsPrimitive::IndicesType::UnsignedInt, 0));
	drawable->VertexArray = vao;

	return drawable;
}