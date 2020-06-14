//This file is something like sandbox. It is just functionality test, not example.

#include <AT2/camera.h>
#include <AT2/MeshLoader.h>
#include <AT2/Scene.h>
#include <AT2/TextureLoader.h>
#include <AT2/OpenGL/GlRenderer.h>
#include <AT2/OpenGL/GlTimerQuery.h>
#include <AT2/OpenGL/GlUniformBuffer.h>
#include <AT2/OpenGL/GLFW/glfw_application.h>
#include <AT2/OpenGL/GLFW/glfw_window.h>

#include <filesystem>
#include <fstream>
#include <iostream>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/quaternion.hpp>

#include "SceneRenderer.h"

constexpr size_t NumActiveLights = 50;

class App
{
public:
	App()
	{
		GlfwApplication::get().OnNoActiveWindows = []
		{
			GlfwApplication::get().stop();
			//spdlog::info("Exit");
		};


		m_window = GlfwApplication::get().createWindow(
			{ GlfwOpenglProfile::Core, 4, 5, 0, 60, false, true }
		);

		m_window->
			setLabel("Some engine test").
			setSize({1024, 768}).
			setCursorMode(GlfwCursorMode::Disabled);


		SetupWindowCallbacks();
	}

	void Run()
	{
		GlfwApplication::get().run();
	}

private:
	void OnInitialize()
	{
		glewExperimental = GL_TRUE;
		if (glewInit() != GLEW_OK)
			throw GlfwException("Failed to initialize GLEW"); //yes, it's strange to throw a Glfw exception :3

		m_renderer = std::make_unique<AT2::GlRenderer>();

		TerrainShader = m_renderer->GetResourceFactory().CreateShaderProgramFromFiles({
			"resources/shaders/terrain.vs.glsl",
			"resources/shaders/terrain.tcs.glsl",
			"resources/shaders/terrain.tes.glsl",
			"resources/shaders/terrain.fs.glsl" });


		MeshShader = m_renderer->GetResourceFactory().CreateShaderProgramFromFiles({
			"resources/shaders/mesh.vs.glsl",
			"resources/shaders/mesh.fs.glsl" });


		Noise3Tex = m_renderer->GetResourceFactory().CreateTexture(Texture3D{ {64, 64, 64}, 1 }, TextureFormats::RGBA8);
		{
			const int l = 64 * 64 * 64 * 4;
			auto arr = std::make_unique<GLubyte[]>(l); //TODO: make basic texture data available without std::holds_alternative =/
			for (size_t i = 0; i < l; ++i)
				arr[i] = (rand() & 0xFF);
			Noise3Tex->SubImage3D({ 0, 0, 0 }, { 64, 64, 64 }, 0, TextureFormats::RGBA8, arr.get());
		}

		GrassTex = AT2::TextureLoader::LoadTexture(m_renderer, "resources/Ground037_2K-JPG/Ground037_2K_Color.jpg");
		NormalMapTex = AT2::TextureLoader::LoadTexture(m_renderer, "resources/Ground037_2K-JPG/Ground037_2K_Normal.jpg");
		RockTex = AT2::TextureLoader::LoadTexture(m_renderer, "resources/rock04.dds");

		HeightMapTex = AT2::TextureLoader::LoadTexture(m_renderer, "resources/heightmap.dds");
		EnvironmentMapTex = AT2::TextureLoader::LoadTexture(m_renderer, "resources/04-23_Day_D.hdr");

		//LightUB = std::make_shared<AT2::GlUniformBuffer>(std::dynamic_pointer_cast<AT2::GlShaderProgram>(SphereLightShader)->GetUniformBlockInfo("LightingBlock"));

		auto lightsRoot = std::make_shared<Node>("lights"s);
		Scene.GetRoot().AddChild(lightsRoot);

		for (size_t i = 0; i < NumActiveLights; ++i)
		{
			lightsRoot->AddChild(std::make_shared<LightNode>(
				SphereLight{ }, 
				linearRand(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f)) * 10000.0f
			)).SetTransform(glm::translate(glm::mat4{1.0}, { glm::linearRand(-5000.0, 5000.0), glm::linearRand(-300.0, 100.0), glm::linearRand(-5000.0, 5000.0) }));
		}

//		LightsArray[0]->SetUniform("u_lightColor", glm::vec3(0.3f, 0.4f, 1.0f));

		//Init
		glEnable(GL_BLEND);
		//glDisable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_CULL_FACE);


		//Scene
		auto mesh = AT2::MeshLoader::LoadNode(m_renderer, "resources/matball.glb", MeshShader);
		mesh->SetTransform( glm::scale(glm::translate(mesh->GetTransform(), { 0, -140, 0 }), { 10, 10, 10 }));
		Scene.GetRoot().AddChild(std::move(mesh));


		auto TerrainNode = MakeTerrain(*m_renderer, TerrainShader, 64, 64);
		TerrainNode->SetTransform(glm::scale(glm::mat4{ 1.0 }, { 10000, 800, 10000}));
		TerrainNode->GetMesh().Submeshes[0].Textures = { Noise3Tex, HeightMapTex, NormalMapTex, RockTex, GrassTex };
		{
			auto uniformStorage = TerrainNode->GetMesh().Submeshes[0].UniformBuffer;
			uniformStorage->SetUniform("u_texHeight", HeightMapTex);
			uniformStorage->SetUniform("u_texNormalMap", NormalMapTex);
			uniformStorage->SetUniform("u_texGrass", GrassTex);
			uniformStorage->SetUniform("u_texRock", RockTex);
		}

		Scene.GetRoot().AddChild(std::move(TerrainNode));

		sr.Initialize(m_renderer);
	}

	void OnRender(double dt)
	{
		if (NeedResourceReload)
		{
			m_renderer->GetResourceFactory().ReloadResources(AT2::ReloadableGroup::Shaders);
			NeedResourceReload = false;
		}

		//if (MovingLightMode)
		//	LightsArray[0]->SetUniform("u_lightPos", glm::vec4(m_camera.getPosition(), 1.0));

        GlTimerQuery glTimer;
        glTimer.Begin();
        sr.RenderScene(Scene, m_camera, m_renderer->GetDefaultFramebuffer(), Time);
        glTimer.End();

        const double frameTime = glTimer.WaitForResult() * 0.000001; // in ms
        if (floor(Time) < floor(Time + dt))
            Log::Debug() << "Frame time: " << frameTime << std::endl;

        m_renderer->FinishFrame();
	}


	void SetupWindowCallbacks()
	{


		m_window->KeyDownCallback = [&](int key)
		{
			std::cout << "Key " << key << " down" << std::endl;

			if (key == GLFW_KEY_Z)
				WireframeMode = !WireframeMode;
			else if (key == GLFW_KEY_M)
				MovingLightMode = !MovingLightMode;
			else if (key == GLFW_KEY_R)
				NeedResourceReload = true;
		};

		m_window->ResizeCallback = [&](const glm::ivec2& newSize)
		{
			m_camera.setProjection(glm::perspectiveFov(glm::radians(90.0f), static_cast<float>(m_window->getSize().x), static_cast<float>(m_window->getSize().y), 1.0f, 10000.0f));
			sr.ResizeFramebuffers(newSize);
		};

		m_window->MouseUpCallback = [](int key)
		{
			std::cout << "Mouse " << key << std::endl;
		};

		m_window->MouseMoveCallback = [&](const MousePos& pos)
		{
			const auto relativePos = pos.getPos() / static_cast<glm::dvec2>(m_window->getSize());

			m_camera.setRotation(
				glm::angleAxis(glm::mix(-glm::pi<float>(), glm::pi<float>(), relativePos.x), glm::vec3{ 0.0, -1.0, 0.0 }) *
				glm::angleAxis(glm::mix(-glm::pi<float>() / 2, glm::pi<float>() / 2, relativePos.y), glm::vec3{ 1.0, 0.0, 0.0 })
			);
		};

		m_window->InitializeCallback = [&]()
		{
			m_window->setVSyncInterval(1);
		};

		m_window->ClosingCallback = [&]()
		{
			m_renderer->Shutdown();
		};

		m_window->UpdateCallback = [&](double dt)
		{
			Time += dt;

			if (m_window->isKeyDown(GLFW_KEY_LEFT_SHIFT))
                acceleration = std::min(acceleration + static_cast<float>(dt), 200.0f);
            else
                acceleration *= 0.98;

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
		};

		m_window->RenderCallback = std::bind(&App::OnRender, this, std::placeholders::_1);
		m_window->InitializeCallback = std::bind(&App::OnInitialize, this);
	}

private:
    std::shared_ptr<GlfwWindow> m_window;
    std::shared_ptr<AT2::IRenderer> m_renderer;

    AT2::Camera m_camera;


    std::shared_ptr<AT2::IShaderProgram> MeshShader, TerrainShader;
    std::shared_ptr<AT2::ITexture> Noise3Tex, HeightMapTex, NormalMapTex, RockTex, GrassTex, EnvironmentMapTex;

    Scene Scene;
    SceneRenderer sr;

    bool WireframeMode = false, MovingLightMode = true, NeedResourceReload = true;
    double Time = 0.0;
    float acceleration = 0.0;

    std::vector<std::shared_ptr<AT2::GlUniformBuffer>> LightsArray;
};

int main(int argc, char *argv[])
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
