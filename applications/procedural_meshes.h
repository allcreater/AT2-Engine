#include <Mesh.h>

namespace AT2
{
    struct Mesh;

    namespace Scene
    {
        class Node;
    }
} // namespace AT2

namespace AT2::Utils
{
    std::unique_ptr<Scene::Node> MakeTerrain(const IVisualizationSystem& renderer, glm::uvec2 numPatches);
    std::unique_ptr<Mesh> MakeSphere(const IVisualizationSystem& renderer, glm::uvec2 numPatches);
    std::unique_ptr<Mesh> MakeFullscreenQuadMesh(const IVisualizationSystem& renderer);
} // namespace AT2::Utils