#include "drawable.h"

#include <AT2/OpenGL/GlDrawPrimitive.h> //Unfortunately right now it don't be created abstractrly

std::shared_ptr<AT2::MeshDrawable> AT2::MeshDrawable::MakeSphereDrawable(const IRenderer& renderer, int segX, int segY)
{
    assert(segX <= 1024 && segY <= 512);

    std::vector<glm::vec3> normals;
    normals.reserve(segX * segY);
    std::vector<glm::uint> indices;
    indices.reserve(segX * segY * 6);

    for (int j = 0; j < segY; ++j)
    {
        const double angV = j * pi / (segY - 1);
        for (int i = 0; i < segX; ++i)
        {
            const double angH = i * pi * 2 / segX;

            normals.push_back(glm::vec3(sin(angV) * cos(angH), sin(angV) * sin(angH), cos(angV)));
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

    auto& rf = renderer.GetResourceFactory();

    auto vao = rf.CreateVertexArray();
    vao->SetVertexBuffer(1,
                         rf.CreateVertexBuffer(AT2vbt::ArrayBuffer, normals.size() * sizeof(glm::vec3), normals.data()),
                         AT2::BufferDataTypes::Vec3);
    vao->SetIndexBuffer(rf.CreateVertexBuffer(AT2vbt::IndexBuffer, indices.size() * sizeof(glm::uint), indices.data()),
                        AT2::BufferDataType::UInt);

    auto drawable = std::make_shared<AT2::MeshDrawable>();
    drawable->Primitives.push_back(
        new AT2::GlDrawElementsPrimitive(AT2::GlDrawPrimitiveType::Triangles, indices.size(),
                                         AT2::GlDrawElementsPrimitive::IndicesType::UnsignedInt, 0));
    drawable->VertexArray = vao;

    return drawable;
}

std::shared_ptr<AT2::MeshDrawable> AT2::MeshDrawable::MakeFullscreenQuadDrawable(const IRenderer& renderer)
{
    static glm::vec3 positions[] = {glm::vec3(-1.0, -1.0, -1.0), glm::vec3(1.0, -1.0, -1.0), glm::vec3(1.0, 1.0, -1.0),
                                    glm::vec3(-1.0, 1.0, -1.0)};

    auto& rf = renderer.GetResourceFactory();

    auto vao = rf.CreateVertexArray();
    vao->SetVertexBuffer(1, rf.CreateVertexBuffer(AT2vbt::ArrayBuffer, 4 * sizeof(glm::vec3), positions),
                         BufferDataTypes::Vec3);

    auto drawable = std::make_shared<AT2::MeshDrawable>();
    drawable->Primitives.push_back(new AT2::GlDrawArraysPrimitive(AT2::GlDrawPrimitiveType::TriangleFan, 0, 4));
    drawable->VertexArray = vao;

    return drawable;
}
