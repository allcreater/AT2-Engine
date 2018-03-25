#include <AT2/OpenGl/GlRenderer.h>
#include <AT2/OpenGl/GlShaderProgram.h>
#include <AT2/OpenGl/GlUniformBuffer.h>
#include <AT2/OpenGl/GlTexture.h>
#include <AT2/OpenGl/GlVertexArray.h>
#include <AT2/OpenGl/GlFrameBuffer.h>
#include <AT2/OpenGl/GlUniformContainer.h>
#include <AT2/OpenGl/GlTimerQuery.h>
#include <AT2/OpenGL/GLFW/glfw_window.h>

#include "../drawable.h" //TODO

//#include <AT2/OpenGl/GlDrawPrimitive.h>

#include <iostream>
#include <fstream>

class App
{
public:
	App()
	{
		m_window.setWindowLabel("Some engine test");
		//m_window.setWindowSize(m_framebufferPhysicalSize.x, m_framebufferPhysicalSize.y);

		SetupWindowCallbacks();
	}

	void Run()
	{
		m_window.Run();
	}

private:
	void OnInitialize()
	{
		m_renderer = std::make_unique<AT2::GlRenderer>();

		auto postprocessShader = m_renderer->GetResourceFactory().CreateShaderProgramFromFiles({
			"data\\shaders\\test.vs.glsl",
			"data\\shaders\\test.fs.glsl" });

		m_quad = AT2::MeshDrawable::MakeFullscreenQuadDrawable(m_renderer.get());
		m_quad->Shader = postprocessShader;
		//QuadDrawable->Textures = { Stage2FBO->GetColorAttachement(0), Stage2FBO->GetDepthAttachement(), Noise3Tex, Stage1FBO->GetColorAttachement(0), GrassTex };
		{
			auto uniformStorage = std::make_shared<AT2::GlUniformContainer>(std::dynamic_pointer_cast<AT2::GlShaderProgram>(postprocessShader));
			//uniformStorage->SetUniform("u_phase", Phase);
			//uniformStorage->SetUniform("u_texNoise", Noise3Tex);
			//uniformStorage->SetUniform("u_colorMap", Stage2FBO->GetColorAttachement(0));
			//uniformStorage->SetUniform("u_depthMap", Stage2FBO->GetDepthAttachement());
			m_quad->UniformBuffer = uniformStorage;
		}


		glDisable(GL_CULL_FACE);
	}

	void OnRender(double time, double dt)
	{

		glViewport(0, 0, m_window.getWindowSize().x, m_window.getWindowSize().y);

		m_renderer->ClearBuffer(glm::vec4(0.0, 0.0, 1.0, 0.0));
		m_renderer->ClearDepth(1.0);

		m_quad->Draw(*m_renderer.get());

		m_renderer->FinishFrame();
		
	}

	void SetupWindowCallbacks()
	{


		m_window.KeyDownCallback = [&](int key)
		{
			std::cout << "Key " << key << " down" << std::endl;
		};

		m_window.KeyRepeatCallback = [&](int key)
		{
		};

		m_window.ResizeCallback = [&](const glm::ivec2& newSize)
		{
		};

		m_window.MouseUpCallback = [](int key)
		{
			std::cout << "Mouse " << key << std::endl;
		};

		m_window.MouseMoveCallback = [&](const MousePos& pos)
		{

		};

		m_window.InitializeCallback = [&]()
		{
			m_window.setVSyncInterval(1);
		};

		m_window.ClosingCallback = [&]()
		{
			m_renderer->Shutdown();
		};

		m_window.RenderCallback = std::bind(&App::OnRender, this, std::placeholders::_1, std::placeholders::_2);
		m_window.InitializeCallback = std::bind(&App::OnInitialize, this);
	}

private:
	GlfwWindow m_window;
	std::unique_ptr<AT2::GlRenderer> m_renderer;

	std::shared_ptr<AT2::MeshDrawable> m_quad;
};


int main(int argc, char *argv[])
{
	try
	{
		InitGLFW();

		App app;
		app.Run();

		ReleaseGLFW();
	}
	catch (AT2::AT2Exception exception)
	{
		std::cout << "Runtime exception:" << exception.what() << std::endl;
	}

	return 0;
}