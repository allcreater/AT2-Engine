#include <AT2/Mesh.h>

namespace AT2
{
    struct Mesh;
    class MeshNode;
} // namespace AT2

namespace AT2::Utils
{
    std::unique_ptr<MeshNode> MakeTerrain(const IRenderer& renderer, glm::uvec2 numPatches);
    std::unique_ptr<Mesh> MakeSphere(const IRenderer& renderer, glm::uvec2 numPatches);
    std::unique_ptr<Mesh> MakeFullscreenQuadMesh(const IRenderer& renderer);
} // namespace AT2::Utils