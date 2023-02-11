//This file is something like sandbox. It is just functionality test, not example.

#include <Camera.h>
#include <Scene/Scene.h>
//#include <Platform/Renderers/OpenGL/GlTimerQuery.h>
#include <Platform/Application.h>
#include <Resources/MeshLoader.h>
#include <Resources/GltfSceneLoader.h>
#include <Resources/TextureLoader.h>

#include <execution>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>

#include <glm/gtc/random.hpp>

#include "SceneRenderer.h"
#include "../procedural_meshes.h"

using namespace std::literals;

using TextureLoader = AT2::Resources::TextureLoader;
using MeshLoader = AT2::Resources::MeshLoader;

constexpr size_t NumActiveLights = 50;

class Sandbox final : public AT2::WindowContextBase
{
public:
    Sandbox() = default;

private:
    std::shared_ptr<AT2::ITexture> ComputeHeightmap(AT2::IVisualizationSystem& visualizationSystem, glm::uvec2 resolution) const
    {
        constexpr auto localGroupSize = glm::uvec3 {32, 32, 1};

        auto resultTex =
            visualizationSystem.GetResourceFactory().CreateTexture(AT2::Texture2D {AT2::TextureFormat::RGBA16Float, resolution}, false);
        resultTex->SetWrapMode(AT2::TextureWrapParams::Uniform(AT2::TextureWrapMode::ClampToBorder));
        auto shader =
            visualizationSystem.GetResourceFactory().CreateShaderProgramFromFiles({"resources/shaders/generate.cs.glsl"});

        //shader->SetUniform("u_result", 0);

        resultTex->BindAsImage(0, 0, 0, false);
        visualizationSystem.DispatchCompute(shader, glm::uvec3 {resolution, 1} / localGroupSize);

        return resultTex;
    }

    void OnInitialized( AT2::IVisualizationSystem& visualizationSystem ) override
    {
        getWindow().setVSyncInterval(1).setCursorMode(CursorMode::Disabled);

        TerrainShader = visualizationSystem.GetResourceFactory().CreateShaderProgramFromFiles(
            {"resources/shaders/terrain.vs.glsl", "resources/shaders/terrain.tcs.glsl",
             "resources/shaders/terrain.tes.glsl", "resources/shaders/terrain.fs.glsl"});


        MeshShader = visualizationSystem.GetResourceFactory().CreateShaderProgramFromFiles(
            {"resources/shaders/mesh.vs.glsl", "resources/shaders/mesh.fs.glsl"});


        Noise3Tex = visualizationSystem.GetResourceFactory().CreateTexture(AT2::Texture3D {AT2::TextureFormat::RGBA8Unorm, {64, 64, 64}, 1}, false);
        {
            const auto arr = std::make_unique<std::uint8_t[]>(Noise3Tex->GetDataLength());
            
            std::generate(arr.get(), arr.get() + Noise3Tex->GetDataLength(),
                          [rng = std::mt19937{std::random_device {}()}]() mutable {
                              return std::uniform_int_distribution {0, 255}(rng);
                          });
            
            Noise3Tex->SubImage3D({}, Noise3Tex->GetSize(), 0, AT2::TextureFormat::RGBA8Unorm, arr.get());
        }

        auto GrassTex = TextureLoader::LoadTexture(visualizationSystem, "resources/Ground037_2K-JPG/Ground037_2K_Color.jpg");
        auto NormalMapTex = TextureLoader::LoadTexture(visualizationSystem, "resources/Ground037_2K-JPG/Ground037_2K_Normal.jpg");
        //auto RockTex = TextureLoader::LoadTexture(visualizationSystem, "resources/Rock035_2K-JPG/Rock035_2K_Color.jpg");
        //auto RockNormalTex = TextureLoader::LoadTexture(visualizationSystem, "resources/Rock035_2K-JPG/Rock035_2K_Normal.jpg");
        //auto RockDisplacementTex = TextureLoader::LoadTexture(visualizationSystem, "resources/Rock035_2K-JPG/Rock035_2K_Displacement.jpg");

        HeightMapTex = ComputeHeightmap(visualizationSystem, glm::uvec2 {8192});
        EnvironmentMapTex = TextureLoader::LoadTexture(visualizationSystem, "resources/04-23_Day_D.hdr");

        auto lightsRoot = std::make_shared<AT2::Scene::Node>("lights"s);
        m_scene.GetRoot().AddChild(lightsRoot);

        for (size_t i = 0; i < NumActiveLights; ++i)
        {
            lightsRoot
                ->AddChild(std::make_shared<AT2::Scene::LightNode>(
                    AT2::Scene::SphereLight {}, linearRand(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f)) * 10000.0f,
                    "PointLight[" + std::to_string(i) + "]"))
                .SetTransform(glm::translate(glm::mat4 {1.0},
                                             {glm::linearRand(-5000.0, 5000.0), glm::linearRand(-300.0, 100.0),
                                              glm::linearRand(-5000.0, 5000.0)}));
        }

        lightsRoot->AddChild(std::make_shared<AT2::Scene::LightNode>(AT2::Scene::SkyLight {glm::vec3(0.0f, 0.707f, 0.707f), EnvironmentMapTex},
                                                                     glm::vec3(500.0f), "SkyLight"));

        //m_scene
        //auto matBallNode = MeshLoader::LoadNode(visualizationSystem, "resources/matball.glb");
        //matBallNode->SetTransform(glm::scale(glm::translate(matBallNode->GetTransform().asMatrix(), {100, 50, 0}), {100, 100, 100}));
        //m_scene.GetRoot().AddChild(std::move(matBallNode));

        //auto scene = AT2::Resources::GltfMeshLoader::LoadScene(visualizationSystem, R"(G:\Git\fx-gltf\test\data\glTF-Sample-Models\2.0\RiggedFigure\glTF\RiggedFigure.gltf)"s);
        //auto scene = AT2::Resources::GltfMeshLoader::LoadScene(visualizationSystem, R"(G:\Git\fx-gltf\test\data\glTF-Sample-Models\2.0\CesiumMan\glTF\CesiumMan.gltf)"s);
        //auto scene = AT2::Resources::GltfMeshLoader::LoadScene(visualizationSystem, R"(C:\Users\allcr\Downloads\GLTF\amazing_player_female\scene.gltf)"s);
        //auto scene = AT2::Resources::GltfMeshLoader::LoadScene(visualizationSystem, R"(C:\Users\allcr\Downloads\GLTF\marika\scene.gltf)"s);
        //auto scene = AT2::Resources::GltfMeshLoader::LoadScene(visualizationSystem, R"(G:\Git\fx-gltf\test\data\glTF-Sample-Models\2.0\BrainStem\glTF\BrainStem.gltf)"s);
        //auto scene = AT2::Resources::GltfMeshLoader::LoadScene(visualizationSystem, R"(G:\Git\fx-gltf\test\data\glTF-Sample-Models\2.0\MetalRoughSpheres\glTF\MetalRoughSpheres.gltf)"s);
        //auto scene = AT2::Resources::GltfMeshLoader::LoadScene(visualizationSystem, R"(G:\Git\fx-gltf\test\data\glTF-Sample-Models\2.0\SciFiHelmet\glTF\SciFiHelmet.gltf)"s);

        try
        {
            auto scene = AT2::Resources::GltfMeshLoader::LoadScene(
                visualizationSystem, R"(../glTF-Sample-Models/2.0/Sponza/glTF/Sponza.gltf)"s);
            scene->GetTransform().setScale({20.0, 20.0, 20.0}).setPosition({1000, 500, 0});
            m_scene.GetRoot().AddChild(scene);
        }
        catch (const AT2::AT2IOException& exception)
        {
            std::cout << exception.what() << std::endl;
        }
        //auto scene = AT2::Resources::GltfMeshLoader::LoadScene(visualizationSystem, R"(G:\Git\fx-gltf\test\data\glTF-Sample-Models\2.0\Fox\glTF\Fox.gltf)"s);
        //auto scene = AT2::Resources::GltfMeshLoader::LoadScene(visualizationSystem, R"(G:\Git\fx-gltf\test\data\glTF-Sample-Models\2.0\BoxAnimated\glTF\BoxAnimated.gltf)"s);
        //auto scene = AT2::Resources::GltfMeshLoader::LoadScene(visualizationSystem, R"(G:\Git\fx-gltf\test\data\glTF-Sample-Models\2.0\InterpolationTest\glTF\InterpolationTest.gltf)"s);
        //auto scene = AT2::Resources::GltfMeshLoader::LoadScene(visualizationSystem, R"(G:\Git\fx-gltf\test\data\glTF-Sample-Models\2.0\TriangleWithoutIndices\glTF\TriangleWithoutIndices.gltf)"s);

        //scene->GetTransform().setPosition({0, -20.0, 0});
        //m_scene.GetRoot().AddChild(scene);

        auto opaqueStateDescriptor = AT2::PipelineStateDescriptor()
                                         .SetShader(MeshShader)
                                         .SetDepthState({AT2::CompareFunction::Less, true, true});

        AT2::Scene::SceneRenderer::VisitAllMeshes(m_scene, [&](const AT2::MeshRef& mesh) {
            mesh->PipelineState = visualizationSystem.GetResourceFactory().CreatePipelineState(opaqueStateDescriptor.SetShader(MeshShader));    
        });


        auto terrainNode = AT2::Utils::MakeTerrain(visualizationSystem, glm::vec2(HeightMapTex->GetSize()) / glm::vec2(64));
        {
            terrainNode->SetTransform(glm::scale(glm::mat4 {1.0}, {10000, 800, 10000}));

            auto mesh = terrainNode->getComponent<AT2::Scene::MeshComponent>()->getMesh();
            mesh->PipelineState = visualizationSystem.GetResourceFactory().CreatePipelineState(opaqueStateDescriptor.SetShader(TerrainShader));
            mesh->GetOrCreateDefaultMaterial().Commit([&](AT2::IUniformsWriter& writer) {
                writer.Write("u_texNoise", Noise3Tex);
                writer.Write("u_texHeight", HeightMapTex);
                writer.Write("u_texNormalMap", NormalMapTex);
                writer.Write("u_texGrass", GrassTex);
                writer.Write("u_texRock", GrassTex);
            });
        }
        m_scene.GetRoot().AddChild(std::move(terrainNode));

        m_renderParameters.Scene = &m_scene;
        m_renderParameters.Camera = &m_camera;
        m_renderParameters.TargetFramebuffer = &visualizationSystem.GetDefaultFramebuffer();

        sr.Initialize(visualizationSystem);
    }

    void OnRender( AT2::Seconds dt, AT2::IVisualizationSystem& visualizationSystem )
    {
        if (NeedResourceReload)
        {
            std::cout << "Reloading shaders... " << std::endl;
            visualizationSystem.GetResourceFactory().ReloadResources(AT2::ReloadableGroup::Shaders);
            NeedResourceReload = false;
        }

        visualizationSystem.GetDefaultFramebuffer().Render([this](AT2::IRenderer& renderer){ sr.RenderScene(renderer, m_renderParameters, m_time);});

//        const double frameTime = glTimer.WaitForResult() * 0.000001; // in ms
//        getWindow().setLabel("Frame time = " + std::to_string(frameTime));
    }


    void OnKeyDown(int key) override
    {
        std::cout << "Key " << key << " down" << std::endl;

        if (key == AT2::Keys::Key_Z)
            m_renderParameters.Wireframe = !m_renderParameters.Wireframe;
        else if (key == AT2::Keys::Key_M)
            MovingLightMode = !MovingLightMode;
        else if (key == AT2::Keys::Key_R)
            NeedResourceReload = true;
        else if (key == AT2::Keys::Key_L)
        {
            if (auto* skyLight = m_scene.FindNode<AT2::Scene::LightNode>("SkyLight"sv))
                skyLight->SetEnabled(!skyLight->GetEnabled());
        }
    }

    void OnResize(glm::ivec2 newSize) override
    {
        if (newSize.x <= 0 || newSize.y <= 0)
            return;

        m_camera.setProjection(glm::perspectiveFov(glm::radians(90.0f), static_cast<float>(newSize.x),
                                                    static_cast<float>(newSize.y), 0.1f, 20000.0f));
        sr.ResizeFramebuffers(newSize);
    }

    void OnMouseUp(int key) override
    {
        std::cout << "Mouse " << key << std::endl;
    }

    void OnMouseMove(const AT2::MousePos& pos) override
    {
        const auto relativePos = pos.getPos() / static_cast<glm::dvec2>(getWindow().getSize());

        m_camera.setRotation(glm::angleAxis(glm::mix(-glm::pi<float>(), glm::pi<float>(), relativePos.x),
                                            glm::vec3 {0.0, -1.0, 0.0}) *
                                glm::angleAxis(glm::mix(-glm::pi<float>() / 2, glm::pi<float>() / 2, relativePos.y),
                                            glm::vec3 {1.0, 0.0, 0.0}));
    }

    void OnUpdate(AT2::Seconds dt) override
    {
        m_time.Update(dt);
        m_scene.Update(m_time);

        if (getWindow().isKeyDown(AT2::Keys::Key_LShift))
            acceleration = std::min(acceleration + static_cast<float>(dt.count()), 200.0f);
        else
            acceleration *= 0.98f;

        const float moveSpeed = static_cast<float>(dt.count()) * 50.0f + acceleration;
        if (getWindow().isKeyDown(AT2::Keys::Key_W))
            m_camera.setPosition(m_camera.getPosition() + m_camera.getForward() * moveSpeed);
        if (getWindow().isKeyDown(AT2::Keys::Key_S))
            m_camera.setPosition(m_camera.getPosition() - m_camera.getForward() * moveSpeed);
        if (getWindow().isKeyDown(AT2::Keys::Key_A))
            m_camera.setPosition(m_camera.getPosition() + m_camera.getLeft() * moveSpeed);
        if (getWindow().isKeyDown(AT2::Keys::Key_D))
            m_camera.setPosition(m_camera.getPosition() - m_camera.getLeft() * moveSpeed);

        if (getWindow().isKeyDown(AT2::Keys::Key_Escape))
            getWindow().setCloseFlag(true);

		const float expositionSpeed = 1 + 2 * dt.count();
        if (getWindow().isKeyDown(AT2::Keys::Key_Equal))
            m_renderParameters.Exposure *= expositionSpeed;
        if (getWindow().isKeyDown(AT2::Keys::Key_Minus))
            m_renderParameters.Exposure /= expositionSpeed;
        m_renderParameters.Exposure = glm::clamp(m_renderParameters.Exposure, 0.001f, 10.0f);

        if (MovingLightMode)
        {
            if (auto* light = m_scene.FindNode<AT2::Scene::LightNode>("PointLight[0]"sv))
                light->SetTransform(m_camera.getViewInverse());
        }
    }
   
private:
    //TODO: move out of App class
    class Time : public AT2::ITime
    {
        using Clock = std::chrono::steady_clock;
        AT2::Seconds m_timeFromStart {}, m_deltaTime {};

    public:
        void Update(AT2::Seconds dt)
        {
            m_deltaTime = dt;
            m_timeFromStart += m_deltaTime;
        }

        AT2::Seconds getTime() const override { return m_timeFromStart; }
        AT2::Seconds getDeltaTime() const override { return m_deltaTime; }
    } m_time;

    std::shared_ptr<AT2::IShaderProgram> MeshShader, TerrainShader;
    std::shared_ptr<AT2::ITexture> Noise3Tex, HeightMapTex, EnvironmentMapTex;

    AT2::Camera m_camera;
    AT2::Scene::Scene m_scene;
    AT2::Scene::SceneRenderer sr;

    AT2::Scene::RenderParameters m_renderParameters;
    bool MovingLightMode = true, NeedResourceReload = true;
    float acceleration = 0.0;
};

int main(const int argc, const char* argv[])
{
    try
    {
        AT2::SingleWindowApplication app;
        app.Run(std::make_unique<Sandbox>());
    }
    catch (AT2::AT2Exception& exception)
    {
        std::cout << "Runtime exception:" << exception.what() << std::endl;
    }

    return 0;
}
