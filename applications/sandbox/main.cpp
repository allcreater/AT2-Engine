//This file is something like sandbox. It is just functionality test, not example.

#include <camera.h>
#include <Scene/Scene.h>
#include <Platform/Renderers/OpenGL/GlTimerQuery.h>
#include <Platform/Application.h>
#include <Platform/GLFW/glfw_window.h>
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

class Sandbox final : public AT2::GraphicsContext
{
public:
    Sandbox() = default;

private:
    std::shared_ptr<AT2::ITexture> ComputeHeightmap(glm::uvec2 resolution) const
    {
        constexpr auto localGroupSize = glm::uvec3 {32, 32, 1};

        auto resultTex =
            getRenderer()->GetResourceFactory().CreateTexture(Texture2D {resolution}, AT2::TextureFormats::RGBA16F);
        resultTex->SetWrapMode(AT2::TextureWrapParams::Uniform(AT2::TextureWrapMode::ClampToBorder));
        auto shader =
            getRenderer()->GetResourceFactory().CreateShaderProgramFromFiles({"resources/shaders/generate.cs.glsl"});

        getRenderer()->GetStateManager().BindShader(shader);

        shader->SetUniform("u_result", 0);

        resultTex->BindAsImage(0, 0, 0, false);
        getRenderer()->DispatchCompute(glm::uvec3 {resolution, 1} / localGroupSize);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        return resultTex;
    }

    void OnInitialized() override
    {
        getWindow().setVSyncInterval(1).setCursorMode(CursorMode::Disabled);

        TerrainShader = getRenderer()->GetResourceFactory().CreateShaderProgramFromFiles(
            {"resources/shaders/terrain.vs.glsl", "resources/shaders/terrain.tcs.glsl",
             "resources/shaders/terrain.tes.glsl", "resources/shaders/terrain.fs.glsl"});


        MeshShader = getRenderer()->GetResourceFactory().CreateShaderProgramFromFiles(
            {"resources/shaders/mesh.vs.glsl", "resources/shaders/mesh.fs.glsl"});


        Noise3Tex = getRenderer()->GetResourceFactory().CreateTexture(Texture3D {{64, 64, 64}, 1}, AT2::TextureFormats::RGBA8);
        {
            const auto arr = std::make_unique<GLubyte[]>(Noise3Tex->GetDataLength());
            
            std::generate(std::execution::par_unseq, arr.get(), arr.get() + Noise3Tex->GetDataLength(),
                          [rng = std::mt19937{std::random_device {}()}]() mutable {
                              return std::uniform_int_distribution {0, 255}(rng);
                          });
            
            Noise3Tex->SubImage3D({}, Noise3Tex->GetSize(), 0, AT2::TextureFormats::RGBA8, arr.get());
        }

        auto GrassTex = TextureLoader::LoadTexture(getRenderer(), "resources/Ground037_2K-JPG/Ground037_2K_Color.jpg");
        auto NormalMapTex = TextureLoader::LoadTexture(getRenderer(), "resources/Ground037_2K-JPG/Ground037_2K_Normal.jpg");
        auto RockTex = TextureLoader::LoadTexture(getRenderer(), "resources/Ground037_2K-JPG/Ground037_2K_Displacement.jpg");
        //auto RockNormalTex = TextureLoader::LoadTexture(getRenderer(), "resources/Rock035_2K-JPG/Rock035_2K_Normal.jpg");
        //auto RockDisplacementTex = TextureLoader::LoadTexture(getRenderer(), "resources/Rock035_2K-JPG/Rock035_2K_Displacement.jpg");

        HeightMapTex = ComputeHeightmap(glm::uvec2 {8192});
        EnvironmentMapTex = TextureLoader::LoadTexture(getRenderer(), "resources/04-23_Day_D.hdr");

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
        auto matBallNode = MeshLoader::LoadNode(getRenderer(), "resources/matball.glb");
        matBallNode->SetTransform(glm::scale(glm::translate(matBallNode->GetTransform().asMatrix(), {100, 50, 0}), {100, 100, 100}));
        m_scene.GetRoot().AddChild(std::move(matBallNode));

        //auto scene = AT2::Resources::GltfMeshLoader::LoadScene(getRenderer(), R"(G:\Git\fx-gltf\test\data\glTF-Sample-Models\2.0\RiggedFigure\glTF\RiggedFigure.gltf)"s);
        //auto scene = AT2::Resources::GltfMeshLoader::LoadScene(getRenderer(), R"(G:\Git\fx-gltf\test\data\glTF-Sample-Models\2.0\CesiumMan\glTF\CesiumMan.gltf)"s);
        //auto scene = AT2::Resources::GltfMeshLoader::LoadScene(getRenderer(), R"(C:\Users\allcr\Downloads\GLTF\amazing_player_female\scene.gltf)"s);
        //auto scene = AT2::Resources::GltfMeshLoader::LoadScene(getRenderer(), R"(C:\Users\allcr\Downloads\GLTF\marika\scene.gltf)"s);
        //auto scene = AT2::Resources::GltfMeshLoader::LoadScene(getRenderer(), R"(G:\Git\fx-gltf\test\data\glTF-Sample-Models\2.0\BrainStem\glTF\BrainStem.gltf)"s);
        //auto scene = AT2::Resources::GltfMeshLoader::LoadScene(getRenderer(), R"(G:\Git\fx-gltf\test\data\glTF-Sample-Models\2.0\MetalRoughSpheres\glTF\MetalRoughSpheres.gltf)"s);
        //auto scene = AT2::Resources::GltfMeshLoader::LoadScene(getRenderer(), R"(G:\Git\fx-gltf\test\data\glTF-Sample-Models\2.0\SciFiHelmet\glTF\SciFiHelmet.gltf)"s);

        //castle
        //{
        //    auto scene = AT2::Resources::GltfMeshLoader::LoadScene(
        //        getRenderer(), R"(G:\Git\fx-gltf\test\data\glTF-Sample-Models\2.0\Sponza\glTF\Sponza.gltf)"s);
        //    scene->GetTransform().setScale({20.0, 20.0, 20.0}).setPosition({1000, 300, 0});
        //    m_scene.GetRoot().AddChild(scene);
        //}
        //auto scene = AT2::Resources::GltfMeshLoader::LoadScene(getRenderer(), R"(G:\Git\fx-gltf\test\data\glTF-Sample-Models\2.0\Fox\glTF\Fox.gltf)"s);
        //auto scene = AT2::Resources::GltfMeshLoader::LoadScene(getRenderer(), R"(G:\Git\fx-gltf\test\data\glTF-Sample-Models\2.0\BoxAnimated\glTF\BoxAnimated.gltf)"s);
        //auto scene = AT2::Resources::GltfMeshLoader::LoadScene(getRenderer(), R"(G:\Git\fx-gltf\test\data\glTF-Sample-Models\2.0\InterpolationTest\glTF\InterpolationTest.gltf)"s);
        //auto scene = AT2::Resources::GltfMeshLoader::LoadScene(getRenderer(), R"(G:\Git\fx-gltf\test\data\glTF-Sample-Models\2.0\TriangleWithoutIndices\glTF\TriangleWithoutIndices.gltf)"s);

        //scene->GetTransform().setPosition({0, -20.0, 0});
        //m_scene.GetRoot().AddChild(scene);

        AT2::Scene::FuncNodeVisitor shaderSetter {[&](AT2::Scene::Node& node) {
            for (auto* meshComponent : node.getComponents<AT2::Scene::MeshComponent>())
                meshComponent->getMesh()->Shader = MeshShader;
            return true;
        }};
        m_scene.GetRoot().Accept(shaderSetter);


        auto terrainNode = AT2::Utils::MakeTerrain(*getRenderer(), glm::vec2(HeightMapTex->GetSize()) / glm::vec2(64));
        {
            terrainNode->SetTransform(glm::scale(glm::mat4 {1.0}, {10000, 800, 10000}));

            auto mesh = terrainNode->getComponent<AT2::Scene::MeshComponent>()->getMesh();
            mesh->Shader = TerrainShader;
            auto& uniformStorage = mesh->GetOrCreateDefaultMaterial();
            uniformStorage.SetUniform("u_texNoise", Noise3Tex);
            uniformStorage.SetUniform("u_texHeight", HeightMapTex);
            uniformStorage.SetUniform("u_texNormalMap", NormalMapTex);
            uniformStorage.SetUniform("u_texGrass", GrassTex);
            uniformStorage.SetUniform("u_texRock", RockTex);
        }
        m_scene.GetRoot().AddChild(std::move(terrainNode));

        m_renderParameters.Scene = &m_scene;
        m_renderParameters.Camera = &m_camera;
        m_renderParameters.TargetFramebuffer = nullptr;

        sr.Initialize(getRenderer());
    }

    void OnRender(AT2::Seconds dt)
    {
        if (NeedResourceReload)
        {
            std::cout << "Reloading shaders... " << std::endl;
            getRenderer()->GetResourceFactory().ReloadResources(AT2::ReloadableGroup::Shaders);
            NeedResourceReload = false;
        }

        AT2::OpenGL::GlTimerQuery glTimer;
        glTimer.Begin();
        sr.RenderScene(m_renderParameters, m_time);
        glTimer.End();

        const double frameTime = glTimer.WaitForResult() * 0.000001; // in ms
        getWindow().setLabel("Frame time = " + std::to_string(frameTime));

        getRenderer()->FinishFrame();
    }


    void OnKeyDown(int key) override
    {
        std::cout << "Key " << key << " down" << std::endl;

        if (key == GLFW_KEY_Z)
            m_renderParameters.Wireframe = !m_renderParameters.Wireframe;
        else if (key == GLFW_KEY_M)
            MovingLightMode = !MovingLightMode;
        else if (key == GLFW_KEY_R)
            NeedResourceReload = true;
        else if (key == GLFW_KEY_L)
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
    };

    void OnMouseMove(const AT2::MousePos& pos) override
    {
        const auto relativePos = pos.getPos() / static_cast<glm::dvec2>(getWindow().getSize());

        m_camera.setRotation(glm::angleAxis(glm::mix(-glm::pi<float>(), glm::pi<float>(), relativePos.x),
                                            glm::vec3 {0.0, -1.0, 0.0}) *
                                glm::angleAxis(glm::mix(-glm::pi<float>() / 2, glm::pi<float>() / 2, relativePos.y),
                                            glm::vec3 {1.0, 0.0, 0.0}));
    }

    void OnClosing() override
    { 
        getRenderer()->Shutdown(); 
    }

    void OnUpdate(AT2::Seconds dt) override
    {
        m_time.Update(dt);
        m_scene.Update(m_time);

        if (getWindow().isKeyDown(GLFW_KEY_LEFT_SHIFT))
            acceleration = std::min(acceleration + static_cast<float>(dt.count()), 200.0f);
        else
            acceleration *= 0.98f;

        const float moveSpeed = static_cast<float>(dt.count()) * 50.0f + acceleration;
        if (getWindow().isKeyDown(GLFW_KEY_W))
            m_camera.setPosition(m_camera.getPosition() + m_camera.getForward() * moveSpeed);
        if (getWindow().isKeyDown(GLFW_KEY_S))
            m_camera.setPosition(m_camera.getPosition() - m_camera.getForward() * moveSpeed);
        if (getWindow().isKeyDown(GLFW_KEY_A))
            m_camera.setPosition(m_camera.getPosition() + m_camera.getLeft() * moveSpeed);
        if (getWindow().isKeyDown(GLFW_KEY_D))
            m_camera.setPosition(m_camera.getPosition() - m_camera.getLeft() * moveSpeed);

        if (getWindow().isKeyDown(GLFW_KEY_ESCAPE))
            getWindow().setCloseFlag(true);

		const float expositionSpeed = 1 + 2 * dt.count();
        if (getWindow().isKeyDown(GLFW_KEY_EQUAL))
            m_renderParameters.Exposure *= expositionSpeed;
        if (getWindow().isKeyDown(GLFW_KEY_MINUS))
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
