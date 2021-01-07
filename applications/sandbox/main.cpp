//This file is something like sandbox. It is just functionality test, not example.

#include <AT2/camera.h>
#include <AT2/Scene.h>
#include <AT2/OpenGL/GlRenderer.h>
#include <AT2/OpenGL/GlTimerQuery.h>
#include <AT2/OpenGL/GLFW/glfw_application.h>
#include <AT2/OpenGL/GLFW/glfw_window.h>
#include <AT2/Resources/MeshLoader.h>
#include <AT2/Resources/GltfSceneLoader.h>
#include <AT2/Resources/TextureLoader.h>

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

class App
{
public:
    App()
    {
        GlfwApplication::get().OnNoActiveWindows = [] {
            GlfwApplication::get().stop();
            //spdlog::info("Exit");
        };

        m_window = GlfwApplication::get().createFullscreenWindow({GlfwOpenglProfile::Core, 4, 5, 0, 60, false, true});
        m_window->setLabel("Some engine test").setCursorMode(GlfwCursorMode::Disabled);

        SetupWindowCallbacks();
    }

    void Run() { GlfwApplication::get().run(); }

private:
    std::shared_ptr<AT2::ITexture> ComputeHeightmap(glm::uvec2 resolution) const
    {
        constexpr auto localGroupSize = glm::uvec3 {32, 32, 1};

        auto resultTex =
            m_renderer->GetResourceFactory().CreateTexture(Texture2D {resolution}, AT2::TextureFormats::RGBA16F);
        resultTex->SetWrapMode(AT2::TextureWrapMode::ClampToBorder);
        auto shader =
            m_renderer->GetResourceFactory().CreateShaderProgramFromFiles({"resources/shaders/generate.cs.glsl"});

        m_renderer->GetStateManager().BindShader(shader);

        shader->SetUniform("u_result", 0);

        resultTex->BindAsImage(0, 0, 0, false);
        m_renderer->DispatchCompute(glm::uvec3 {resolution, 1} / localGroupSize);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        return resultTex;
    }

    void OnInitialize()
    {
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK)
            throw GlfwException("Failed to initialize GLEW"); //yes, it's strange to throw a Glfw exception :3

        m_renderer = std::make_unique<AT2::GlRenderer>();

        TerrainShader = m_renderer->GetResourceFactory().CreateShaderProgramFromFiles(
            {"resources/shaders/terrain.vs.glsl", "resources/shaders/terrain.tcs.glsl",
             "resources/shaders/terrain.tes.glsl", "resources/shaders/terrain.fs.glsl"});


        MeshShader = m_renderer->GetResourceFactory().CreateShaderProgramFromFiles(
            {"resources/shaders/mesh.vs.glsl", "resources/shaders/mesh.fs.glsl"});


        Noise3Tex = m_renderer->GetResourceFactory().CreateTexture(Texture3D {{64, 64, 64}, 1}, AT2::TextureFormats::RGBA8);
        {
            const auto arr = std::make_unique<GLubyte[]>(Noise3Tex->GetDataLength());

            std::generate(std::execution::par_unseq, arr.get(), arr.get() + Noise3Tex->GetDataLength(),
                          [rng = std::mt19937{std::random_device {}()}]() mutable {
                              return std::uniform_int_distribution {0, 255}(rng);
                          });
            
            Noise3Tex->SubImage3D({}, Noise3Tex->GetSize(), 0, AT2::TextureFormats::RGBA8, arr.get());
        }

        GrassTex = TextureLoader::LoadTexture(m_renderer, "resources/Ground037_2K-JPG/Ground037_2K_Color.jpg");
        NormalMapTex =
            TextureLoader::LoadTexture(m_renderer, "resources/Ground037_2K-JPG/Ground037_2K_Normal.jpg");
        RockTex =TextureLoader::LoadTexture(m_renderer, "resources/rock04.dds");

        HeightMapTex = ComputeHeightmap(glm::uvec2 {8192});
        EnvironmentMapTex = TextureLoader::LoadTexture(m_renderer, "resources/04-23_Day_D.hdr");

        auto lightsRoot = std::make_shared<AT2::Node>("lights"s);
        Scene.GetRoot().AddChild(lightsRoot);

        for (size_t i = 0; i < NumActiveLights; ++i)
        {
            lightsRoot
                ->AddChild(std::make_shared<AT2::LightNode>(
                    AT2::SphereLight {}, linearRand(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f)) * 10000.0f,
                    "PointLight[" + std::to_string(i) + "]"))
                .SetTransform(glm::translate(glm::mat4 {1.0},
                                             {glm::linearRand(-5000.0, 5000.0), glm::linearRand(-300.0, 100.0),
                                              glm::linearRand(-5000.0, 5000.0)}));
        }

        lightsRoot->AddChild(std::make_shared<AT2::LightNode>(AT2::SkyLight {glm::vec3(0.0f, 0.707f, 0.707f), EnvironmentMapTex},
                                                              glm::vec3(500.0f), "SkyLight"));

        //Scene
        auto matBallNode = MeshLoader::LoadNode(m_renderer, "resources/matball.glb");
        matBallNode->SetTransform(glm::scale(glm::translate(matBallNode->GetTransform().asMatrix(), {100, 50, 0}), {100, 100, 100}));
        Scene.GetRoot().AddChild(std::move(matBallNode));

        AT2::FuncNodeVisitor shaderSetter {[&](AT2::Node& node) {
            if (auto meshNode = dynamic_cast<AT2::MeshNode*>(&node))
                meshNode->GetMesh()->Shader = MeshShader;
            return true;
        }};
        Scene.GetRoot().Accept(shaderSetter);


        auto terrainNode = AT2::Utils::MakeTerrain(*m_renderer, glm::vec2(HeightMapTex->GetSize()) / glm::vec2(64));
        terrainNode->SetTransform(glm::scale(glm::mat4 {1.0}, {10000, 800, 10000}));
        terrainNode->GetMesh()->Shader = TerrainShader;
        {
            auto& uniformStorage = terrainNode->GetMesh()->GetOrCreateDefaultMaterial();
            uniformStorage.SetUniform("u_texNoise", Noise3Tex);
            uniformStorage.SetUniform("u_texHeight", HeightMapTex);
            uniformStorage.SetUniform("u_texNormalMap", NormalMapTex);
            uniformStorage.SetUniform("u_texGrass", GrassTex);
            uniformStorage.SetUniform("u_texRock", RockTex);
        }
        Scene.GetRoot().AddChild(std::move(terrainNode));

        sr.Initialize(m_renderer);
    }

    void OnRender(double dt)
    {
        if (NeedResourceReload)
        {
            std::cout << "Reloading shaders... " << std::endl;
            m_renderer->GetResourceFactory().ReloadResources(AT2::ReloadableGroup::Shaders);
            NeedResourceReload = false;
        }

        AT2::GlTimerQuery glTimer;
        glTimer.Begin();
        sr.RenderScene({Scene, m_camera, m_renderer->GetDefaultFramebuffer(), Time, WireframeMode});
        glTimer.End();

        const double frameTime = glTimer.WaitForResult() * 0.000001; // in ms
        if (floor(Time) < floor(Time + dt))
            AT2::Log::Debug() << "Frame time: " << frameTime << std::endl;

        m_renderer->FinishFrame();
    }


    void SetupWindowCallbacks()
    {
        m_window->KeyDownCallback = [&](int key) {
            std::cout << "Key " << key << " down" << std::endl;

            if (key == GLFW_KEY_Z)
                WireframeMode = !WireframeMode;
            else if (key == GLFW_KEY_M)
                MovingLightMode = !MovingLightMode;
            else if (key == GLFW_KEY_R)
                NeedResourceReload = true;
            else if (key == GLFW_KEY_L)
            {
                if (auto* skyLight = Scene.FindNode<AT2::LightNode>("SkyLight"sv))
                    skyLight->SetEnabled(!skyLight->GetEnabled());
            }
        };

        m_window->ResizeCallback = [&](const glm::ivec2& newSize) {
            if (newSize.x <= 0 || newSize.y <= 0)
                return;

            m_camera.setProjection(glm::perspectiveFov(glm::radians(90.0f), static_cast<float>(newSize.x),
                                                       static_cast<float>(newSize.y), 1.0f, 20000.0f));
            sr.ResizeFramebuffers(newSize);
        };

        m_window->MouseUpCallback = [](int key) { std::cout << "Mouse " << key << std::endl; };

        m_window->MouseMoveCallback = [&](const MousePos& pos) {
            const auto relativePos = pos.getPos() / static_cast<glm::dvec2>(m_window->getSize());

            m_camera.setRotation(glm::angleAxis(glm::mix(-glm::pi<float>(), glm::pi<float>(), relativePos.x),
                                                glm::vec3 {0.0, -1.0, 0.0}) *
                                 glm::angleAxis(glm::mix(-glm::pi<float>() / 2, glm::pi<float>() / 2, relativePos.y),
                                                glm::vec3 {1.0, 0.0, 0.0}));
        };

        m_window->InitializeCallback = [&]() { m_window->setVSyncInterval(1); };

        m_window->ClosingCallback = [&]() { m_renderer->Shutdown(); };

        m_window->UpdateCallback = [&](const double dt) {
            Time += dt;

            if (m_window->isKeyDown(GLFW_KEY_LEFT_SHIFT))
                acceleration = std::min(acceleration + static_cast<float>(dt), 200.0f);
            else
                acceleration *= 0.98f;

            const float moveSpeed = static_cast<float>(dt) * 50.0f + acceleration;
            if (m_window->isKeyDown(GLFW_KEY_W))
                m_camera.setPosition(m_camera.getPosition() + m_camera.getForward() * moveSpeed);
            if (m_window->isKeyDown(GLFW_KEY_S))
                m_camera.setPosition(m_camera.getPosition() - m_camera.getForward() * moveSpeed);
            if (m_window->isKeyDown(GLFW_KEY_A))
                m_camera.setPosition(m_camera.getPosition() + m_camera.getLeft() * moveSpeed);
            if (m_window->isKeyDown(GLFW_KEY_D))
                m_camera.setPosition(m_camera.getPosition() - m_camera.getLeft() * moveSpeed);

            if (m_window->isKeyDown(GLFW_KEY_ESCAPE))
                m_window->setCloseFlag(true);

            if (MovingLightMode)
            {
                if (auto* light = Scene.FindNode<AT2::LightNode>("PointLight[0]"sv))
                    light->SetTransform(m_camera.getViewInverse());
            }
        };

        m_window->RenderCallback = [this](double dt) { OnRender(dt); };
        m_window->InitializeCallback = [this] { OnInitialize(); };
    }

private:
    std::shared_ptr<GlfwWindow> m_window;
    std::shared_ptr<AT2::IRenderer> m_renderer;

    std::shared_ptr<AT2::IShaderProgram> MeshShader, TerrainShader;
    std::shared_ptr<AT2::ITexture> Noise3Tex, HeightMapTex, NormalMapTex, RockTex, GrassTex, EnvironmentMapTex;

    AT2::Camera m_camera;
    AT2::Scene Scene;
    AT2::SceneRenderer sr;

    bool WireframeMode = false, MovingLightMode = true, NeedResourceReload = true;
    double Time = 0.0;
    float acceleration = 0.0;
};

int main(const int argc, const char* argv[])
{
    try
    {
        App app;
        app.Run();
    }
    catch (AT2::AT2Exception& exception)
    {
        std::cout << "Runtime exception:" << exception.what() << std::endl;
    }

    return 0;
}
