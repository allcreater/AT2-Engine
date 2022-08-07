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
    //TODO: unify, make configurable layouts for generic-purpose meshes

    /// Output: node with one mesh, containing rectangular grid of patches
    /// 1 - texture coords
    std::unique_ptr<Scene::Node> MakeTerrain(const IVisualizationSystem& renderer, glm::uvec2 numPatches);

    /// Output: mesh from rectangulars sectors
    /// 1 - normals
    std::unique_ptr<Mesh> MakeSphere(const IVisualizationSystem& renderer, glm::uvec2 numPatches);

    /// Output: mesh of with just one quad
    /// 1 - positions in clip space (xy from -1 to +1, z = -1)
    std::unique_ptr<Mesh> MakeFullscreenQuadMesh(const IVisualizationSystem& renderer);

    /// Output: mesh with cube
    /// 1 - positions
    /// 2 - texture coords
    /// 3 - normals
    std::unique_ptr<Mesh> MakeCubeMesh(AT2::IResourceFactory& resourceFactory);
} // namespace AT2::Utils