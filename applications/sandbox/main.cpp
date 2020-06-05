//This file is something like sandbox. It is just functionality test, not example.

#include <AT2/camera.h>
#include <AT2/MeshLoader.h>
#include <AT2/Scene.h>
#include <AT2/TextureLoader.h>
#include <AT2/OpenGL/GlFrameBuffer.h>
#include <AT2/OpenGL/GlRenderer.h>
#include <AT2/OpenGL/GlShaderProgram.h>
#include <AT2/OpenGL/GlTexture.h>
#include <AT2/OpenGL/GlTimerQuery.h>
#include <AT2/OpenGL/GlUniformBuffer.h>
#include <AT2/OpenGL/GLFW/glfw_application.h>
#include <AT2/OpenGL/GLFW/glfw_window.h>

#include "../drawable.h"

#include <filesystem>
#include <fstream>
#include <iostream>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/quaternion.hpp>


#include "SceneRenderer.h"


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


		m_window = GlfwApplication::get().createWindow();

		m_window->
			setLabel("Some engine test").
			setSize({1024, 768}).
			setCursorMode(GlfwCursorMode::Normal);


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


		PostprocessShader = m_renderer->GetResourceFactory().CreateShaderProgramFromFiles({
			"resources/shaders/postprocess.vs.glsl",
			"resources/shaders/postprocess.fs.glsl" });

		TerrainShader = m_renderer->GetResourceFactory().CreateShaderProgramFromFiles({
			"resources/shaders/terrain.vs.glsl",
			"resources/shaders/terrain.tcs.glsl",
			"resources/shaders/terrain.tes.glsl",
			"resources/shaders/terrain.fs.glsl" });

		SphereLightShader = m_renderer->GetResourceFactory().CreateShaderProgramFromFiles({
			"resources/shaders/spherelight.vs.glsl",
			"resources/shaders/pbr.fs.glsl",
			"resources/shaders/spherelight.fs.glsl" });

		DayLightShader = m_renderer->GetResourceFactory().CreateShaderProgramFromFiles({
			"resources/shaders/skylight.vs.glsl",
			"resources/shaders/pbr.fs.glsl",
			"resources/shaders/skylight.fs.glsl" });

		MeshShader = m_renderer->GetResourceFactory().CreateShaderProgramFromFiles({
			"resources/shaders/mesh.vs.glsl",
			"resources/shaders/mesh.fs.glsl" });

		auto texture = new AT2::GlTexture3D(GL_RGBA8, glm::uvec3(256, 256, 256), 1);
		AT2::GlTexture::BufferData data;
		data.Height = 256;
		data.Width = 256;
		data.Depth = 256;
		auto arr = std::make_unique<GLubyte[]>(data.Height * data.Width * data.Depth * 4);
		for (size_t i = 0; i < data.Height * data.Width * data.Depth * 4; ++i)
			arr[i] = (rand() & 0xFF);
		data.Data = arr.get();
		texture->SetData(0, data);

		Noise3Tex = std::shared_ptr<AT2::ITexture>(texture);

		GrassTex = AT2::TextureLoader::LoadTexture(m_renderer, "resources/grass03.dds");
		RockTex = AT2::TextureLoader::LoadTexture(m_renderer, "resources/rock04.dds");
		NormalMapTex = AT2::TextureLoader::LoadTexture(m_renderer, "resources/terrain_normalmap.dds");
		HeightMapTex = AT2::TextureLoader::LoadTexture(m_renderer, "resources/heightmap.dds");
		EnvironmentMapTex = AT2::TextureLoader::LoadTexture(m_renderer, "resources/04-23_Day_D.hdr");

		CameraUB = std::make_shared<AT2::GlUniformBuffer>(std::dynamic_pointer_cast<AT2::GlShaderProgram>(TerrainShader)->GetUniformBlockInfo("CameraBlock"));
		CameraUB->SetBindingPoint(1);
		LightUB = std::make_shared<AT2::GlUniformBuffer>(std::dynamic_pointer_cast<AT2::GlShaderProgram>(SphereLightShader)->GetUniformBlockInfo("LightingBlock"));





		for (size_t i = 0; i < NumActiveLights; ++i)
		{
			auto light = std::make_shared<AT2::GlUniformBuffer>(std::dynamic_pointer_cast<AT2::GlShaderProgram>(SphereLightShader)->GetUniformBlockInfo("LightingBlock"));

			light->SetUniform("u_lightPos", glm::vec4(glm::linearRand(-5000.0, 5000.0), glm::linearRand(-300.0, 100.0), glm::linearRand(-5000.0, 5000.0), 1.0));
			light->SetUniform("u_lightRadius", glm::linearRand(300.0f, 700.0f)*2.0f);
			light->SetUniform("u_lightColor", glm::linearRand(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f)));
			light->SetBindingPoint(2);
			LightsArray.push_back(light);
		}

		LightsArray[0]->SetUniform("u_lightColor", glm::vec3(1.0f, 0.0f, 0.5f));

		//Init
		glEnable(GL_BLEND);
		//glDisable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_CULL_FACE);


		SphereLightDrawable = AT2::MeshDrawable::MakeSphereDrawable(m_renderer);
		SphereLightDrawable->Shader = SphereLightShader;

		SkylightDrawable = AT2::MeshDrawable::MakeFullscreenQuadDrawable(m_renderer);
		SkylightDrawable->Shader = DayLightShader;

		QuadDrawable = AT2::MeshDrawable::MakeFullscreenQuadDrawable(m_renderer);
		QuadDrawable->Shader = PostprocessShader;

		//Scene
		auto mesh = AT2::MeshLoader::LoadNode(m_renderer, "resources/matball.glb", MeshShader);
		mesh->SetTransform( glm::scale(glm::translate(mesh->GetTransform(), { 0, -140, 0 }), { 10, 10, 10 }));
		Scene.GetRoot().AddChild(std::move(mesh));


		TerrainNode = MakeTerrain(*m_renderer, TerrainShader, 64, 64);
		TerrainNode->GetChild<DrawableNode>(0).Textures = { Noise3Tex, HeightMapTex, NormalMapTex, RockTex, GrassTex };
		{
			auto uniformStorage = TerrainNode->GetChild<DrawableNode>(0).UniformBuffer;
			uniformStorage->SetUniform("u_scaleH", 10000.0f);
			uniformStorage->SetUniform("u_scaleV", 800.0f);
			uniformStorage->SetUniform("u_texHeight", HeightMapTex);
			uniformStorage->SetUniform("u_texNormalMap", NormalMapTex);
			uniformStorage->SetUniform("u_texGrass", GrassTex);
			uniformStorage->SetUniform("u_texRock", RockTex);
		}

		Scene.GetRoot().AddChild(std::move(TerrainNode));
	}

	void OnRender(double dt)
	{
		if (NeedResourceReload)
		{
			m_renderer->GetResourceFactory().ReloadResources(AT2::ReloadableGroup::Shaders);
			NeedResourceReload = false;
		}

		if (NeedFramebufferResize)
		{
			Stage1FBO = std::make_shared<AT2::GlFrameBuffer>(m_renderer->GetRendererCapabilities());
			Stage1FBO->SetColorAttachement(0, std::make_shared<AT2::GlTexture2D>(GL_RGBA8, m_window->getSize()));
			Stage1FBO->SetColorAttachement(1, std::make_shared<AT2::GlTexture2D>(GL_RGBA32F, m_window->getSize()));
			Stage1FBO->SetColorAttachement(2, std::make_shared<AT2::GlTexture2D>(GL_RGBA8, m_window->getSize()));
			Stage1FBO->SetDepthAttachement(std::make_shared<AT2::GlTexture2D>(GL_DEPTH_COMPONENT32F, m_window->getSize()));

			Stage2FBO = std::make_shared<AT2::GlFrameBuffer>(m_renderer->GetRendererCapabilities());
			Stage2FBO->SetColorAttachement(0, std::make_shared<AT2::GlTexture2D>(GL_RGBA32F, m_window->getSize()));
			Stage2FBO->SetDepthAttachement(Stage1FBO->GetDepthAttachement()); //depth is common with previous stage

			SphereLightDrawable->Textures = { Stage1FBO->GetColorAttachement(0), Stage1FBO->GetColorAttachement(1), Stage1FBO->GetColorAttachement(2), Stage1FBO->GetDepthAttachement(), Noise3Tex };
			{
				auto uniformStorage = SphereLightShader->CreateAssociatedUniformStorage();
				uniformStorage->SetUniform("u_texNoise", Noise3Tex);
				uniformStorage->SetUniform("u_colorMap", Stage1FBO->GetColorAttachement(0));
				uniformStorage->SetUniform("u_normalMap", Stage1FBO->GetColorAttachement(1));
				uniformStorage->SetUniform("u_roughnessMetallicMap", Stage1FBO->GetColorAttachement(2));
				uniformStorage->SetUniform("u_depthMap", Stage1FBO->GetDepthAttachement());

				SphereLightDrawable->UniformBuffer = uniformStorage;
			}


			SkylightDrawable->Textures = { Stage1FBO->GetColorAttachement(0), Stage1FBO->GetColorAttachement(1), Stage1FBO->GetColorAttachement(2), Stage1FBO->GetDepthAttachement(), Noise3Tex, EnvironmentMapTex };
			{
				auto uniformStorage = DayLightShader->CreateAssociatedUniformStorage();
				uniformStorage->SetUniform("u_texNoise", Noise3Tex);
				uniformStorage->SetUniform("u_colorMap", Stage1FBO->GetColorAttachement(0));
				uniformStorage->SetUniform("u_normalMap", Stage1FBO->GetColorAttachement(1));
				uniformStorage->SetUniform("u_roughnessMetallicMap", Stage1FBO->GetColorAttachement(2));
				uniformStorage->SetUniform("u_depthMap", Stage1FBO->GetDepthAttachement());
				uniformStorage->SetUniform("u_environmentMap", EnvironmentMapTex);
				SkylightDrawable->UniformBuffer = uniformStorage;
			}

			//Postprocess quad
			QuadDrawable->Textures = { Stage2FBO->GetColorAttachement(0), Stage2FBO->GetDepthAttachement(), Noise3Tex };
			{
				auto uniformStorage = PostprocessShader->CreateAssociatedUniformStorage();
				uniformStorage->SetUniform("u_texNoise", Noise3Tex);
				uniformStorage->SetUniform("u_colorMap", Stage2FBO->GetColorAttachement(0));
				uniformStorage->SetUniform("u_depthMap", Stage2FBO->GetDepthAttachement());
				QuadDrawable->UniformBuffer = uniformStorage;
			}

			NeedFramebufferResize = false;
		}

		m_renderer->SetViewport(AABB2d({ 0, 0 }, m_window->getSize()));


		if (MovingLightMode)
			LightsArray[0]->SetUniform("u_lightPos", glm::vec4(m_camera.getPosition(), 1.0));

		Render(m_renderer, m_renderer->GetDefaultFramebuffer(), m_camera);


		m_renderer->FinishFrame();
	}

	double Render(std::shared_ptr<AT2::IRenderer>& renderer, AT2::IFrameBuffer& framebuffer, const AT2::Camera& camera)
	{
		AT2::GlTimerQuery glTimer;
		glTimer.Begin();

		CameraUB->SetUniform("u_matView", camera.getView());
		CameraUB->SetUniform("u_matInverseView", camera.getViewInverse());
		CameraUB->SetUniform("u_matProjection", camera.getProjection());
		CameraUB->SetUniform("u_matInverseProjection", camera.getProjectionInverse());
		CameraUB->SetUniform("u_matViewProjection", camera.getProjection() * camera.getView());
		CameraUB->SetUniform("u_time", Time);
		CameraUB->Bind();


		//Scene stage
		Stage1FBO->Bind();
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		renderer->ClearBuffer(glm::vec4(0.0, 0.0, 1.0, 0.0));
		renderer->ClearDepth(1.0);

		glPolygonMode(GL_FRONT_AND_BACK, (WireframeMode) ? GL_LINE : GL_FILL);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glDepthMask(GL_TRUE);
		glCullFace(GL_FRONT);
		glPatchParameteri(GL_PATCH_VERTICES, 4);
		//TerrainDrawable->UniformBuffer->SetUniform("u_matNormal", glm::transpose(glm::inverse(glm::mat3(camera.getView()))));
		//TerrainDrawable->Draw(renderer);

		RenderVisitor::RenderScene(Scene, *renderer, camera);


		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		//Light stage
		Stage2FBO->Bind();

		renderer->ClearBuffer(glm::vec4(0.0, 0.0, 0.0, 0.0));
		//glDisable(GL_DEPTH_TEST);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_GREATER);
		glCullFace(GL_BACK);

		for (const auto& light : LightsArray)
		{
			light->SetBindingPoint(2);
			light->Bind();
			SphereLightDrawable->Draw(renderer);
		}

		glCullFace(GL_BACK);
		glDisable(GL_DEPTH_TEST);
		SkylightDrawable->Draw(renderer);

		//Postprocess stage
		framebuffer.Bind();
		//glEnable(GL_FRAMEBUFFER_SRGB);

		glDepthMask(GL_TRUE);
		renderer->ClearBuffer(glm::vec4(0.0, 0.0, 0.0, 0.0));
		renderer->ClearDepth(0);

		glCullFace(GL_BACK);
		glDisable(GL_DEPTH_TEST);
		(dynamic_cast<AT2::GlShaderProgram*>(QuadDrawable->Shader.get()))->SetUBO("CameraBlock", 1);
		QuadDrawable->Draw(renderer);


		glTimer.End();
		double frameTime = glTimer.WaitForResult() * 0.000001; //in ms

		glFinish();

		return frameTime;
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
			NeedFramebufferResize = true;
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

			const float moveSpeed = static_cast<float>(dt) * 50.0f;
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


	std::shared_ptr<AT2::IShaderProgram> MeshShader, TerrainShader, SphereLightShader, DayLightShader, PostprocessShader;

	std::shared_ptr<AT2::GlUniformBuffer> CameraUB, LightUB;
	std::shared_ptr<AT2::ITexture> Noise3Tex, HeightMapTex, NormalMapTex, RockTex, GrassTex, EnvironmentMapTex;

	std::shared_ptr<AT2::GlFrameBuffer> Stage1FBO, Stage2FBO;

	std::shared_ptr<AT2::MeshDrawable> QuadDrawable, SkylightDrawable, SphereLightDrawable;

	std::shared_ptr<AT2::MeshNode> TerrainNode;
	AT2::Scene Scene;

	bool WireframeMode = false, MovingLightMode = true, NeedResourceReload = true, NeedFramebufferResize = true;
	size_t NumActiveLights = 50;

	double Time = 0.0;

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
