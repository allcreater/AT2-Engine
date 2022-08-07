#include "example_cube.h"
#include "example_gltf.h"

//TODO: switchable examples in one application

int main(const int argc, const char* argv[])
{
    try
    {
        AT2::SingleWindowApplication app;
        app.Run(std::make_unique<CubeExample>());
        app.Run(std::make_unique<GltfSceneExample>());
    }
    catch (AT2::AT2Exception& exception)
    {
        std::cout << "Runtime exception:" << exception.what() << std::endl;
    }

    return 0;
}
