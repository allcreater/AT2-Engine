#include "OpenGl\GlRenderer.h"
#include "OpenGl\GlShaderProgram.h"
#include "OpenGl\GlTexture.h"
#include "OpenGl\GlVertexArray.h"

#include <iostream>
#include <fstream>
#include <SDL.h>

#include <gtc\random.hpp>
#include <gtc\quaternion.hpp>
#include <gtx\quaternion.hpp>


std::shared_ptr<AT2::GlShaderProgram> Shader;
std::shared_ptr<AT2::ITexture> Noise3Tex;
std::shared_ptr<AT2::GlVertexArray> VertexArray;
GLuint vao = 0;
GLfloat Phase = 0.0;

glm::mat4 matMW, matProj;

std::string LoadShader(const char* _filename)
{
	std::ifstream t(_filename);
	return std::string((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
}

void Render(AT2::GlRenderer* renderer)
{
	//matMW = glm::lookAt(glm::vec3(-1.0,0.0,0.0), glm::vec3(0.0,0.0,0.0), glm::vec3(0.0, 1.0, 0.0));


	renderer->ClearBuffer(glm::vec4(0.0, 0.0, 1.0, 1.0));

	AT2::TextureSet cloudsTS;
	cloudsTS.insert(Noise3Tex);
	renderer->GetStateManager()->BindTextures(cloudsTS);

	Shader->SetUniform("u_matMW", matMW);
	Shader->SetUniform("u_matInverseMW", glm::inverse(matMW));
	Shader->SetUniform("u_matProj", matProj);
	Shader->SetUniform("u_matInverseProj", glm::inverse(matProj));
	Shader->SetUniform("u_phase", Phase);
	Shader->SetUniform("u_texNoise", Noise3Tex->GetCurrentModule());
	renderer->GetStateManager()->BindShader(Shader);

	//glBindVertexArray(vao);
	renderer->GetStateManager()->BindVertexArray(VertexArray);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	renderer->SwapBuffers();
	

	Phase += 0.0001;
}

int main(int argc, char *argv[])
{
	try
	{
		auto renderer = new AT2::GlRenderer();

		Shader = std::make_shared<AT2::GlShaderProgram>(
			LoadShader("data\\raytrace_sky.vs.glsl"),
			"",
			"",
			"",
			LoadShader("data\\raytrace_sky.fs.glsl"));

		auto texture = new AT2::GlTexture3D(4, GL_RGBA);
		AT2::ITexture::BufferData data;
		data.Height = 256;
		data.Width = 256;
		data.Depth = 256;
		GLubyte* arr = new GLubyte [data.Height * data.Width * data.Depth * 4];
		for (int i = 0; i < data.Height * data.Width * data.Depth * 4; ++i)
		   arr[i] = (rand() & 0xFF);
		data.Data = arr;
		texture->UpdateData(GL_TEXTURE_3D, data);
		delete [] arr;

		Noise3Tex = std::shared_ptr<AT2::ITexture>(texture);

		glm::vec3 positions[] = {glm::vec3(-1.0, -1.0, -1.0), glm::vec3(1.0, -1.0, -1.0), glm::vec3(1.0, 1.0, -1.0), glm::vec3(-1.0, 1.0, -1.0)};
		GLuint indices[] = {0, 1, 2, 0, 2, 3};


		VertexArray = std::make_shared<AT2::GlVertexArray>();
		VertexArray->SetVertexBuffer(1, std::make_shared<AT2::GlVertexBuffer<glm::vec3>>(AT2::GlVertexBufferBase::BufferType::ArrayBuffer, 4, positions));
		VertexArray->SetIndexBuffer(std::make_shared<AT2::GlVertexBuffer<GLuint>>(AT2::GlVertexBufferBase::BufferType::ElementArrayBuffer, 6, indices));
		
		//Init
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);

		glEnable(GL_TEXTURE_1D);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_TEXTURE_3D);

		glViewport(0, 0, 1024, 1024);
		matProj = glm::frustum(0.0, -1.0, 1.0, -1.0, 0.5, 500.0);

		float heading = 0.0f, pitch = 0.0f;
		while (true)
		{
			SDL_Event sdlEvent;
			//SDL_WaitEvent(&sdlEvent);
			while (SDL_PollEvent(&sdlEvent))
			{
				switch (sdlEvent.type)
				{
					case SDL_MOUSEMOTION:
						{
							heading += sdlEvent.motion.xrel * 0.5f;
							pitch += sdlEvent.motion.yrel * 0.5f;
							pitch = glm::clamp(pitch, -90.0f, 90.0f);
							matMW = glm::toMat4(glm::angleAxis(pitch, glm::vec3(1.0, 0.0, 0.0)) * glm::angleAxis(heading, glm::vec3(0.0, 1.0, 0.0)));
						} break;
					case SDL_QUIT:
						goto QuitLabel;
				}
			}
			
			Render(renderer);
		}

QuitLabel:
		renderer->Shutdown();
	}
	catch (AT2::AT2Exception exeption)
	{
		SDL_ShowSimpleMessageBox(SDL_MessageBoxFlags::SDL_MESSAGEBOX_ERROR, "Exception", exeption.what(), 0);
	}
	
	return 0;
}