#include "OpenGl\GlRenderer.h"
#include "OpenGl\GlShaderProgram.h"
#include "OpenGl\GlUniformBuffer.h"
#include "OpenGl\GlTexture.h"
#include "OpenGl\GlVertexArray.h"

#include <iostream>
#include <fstream>
#include <SDL.h>

#include <glm\gtc\random.hpp>
#include <glm\gtc\quaternion.hpp>
#include <glm\gtx\quaternion.hpp>

#include <gli\gli.hpp>

std::shared_ptr<AT2::GlShaderProgram> Shader, TerrainShader;
std::shared_ptr<AT2::ITexture> Noise3Tex, HeightMapTex, NormalMapTex, RockTex, GrassTex;
std::shared_ptr<AT2::GlVertexArray> VertexArray, TerrainVertexArray;
GLuint vao = 0;
GLfloat Phase = 0.0;

glm::mat4 matMW, matProj;

std::string LoadShader(const char* _filename)
{
	std::ifstream t(_filename);
	return std::string((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
}

std::shared_ptr<AT2::ITexture> LoadTexture (const char* _filename)
{
	gli::storage Storage(gli::load_dds(_filename));
	assert(!Storage.empty());

	if (Storage.layers() > 1)
	{
		throw 1; //TODO
	}
	else
	{
		gli::texture2D Texture(Storage);

		AT2::ITexture::BufferData bd;
		bd.Height = Texture.dimensions().x;
		bd.Width = Texture.dimensions().y;
		bd.Depth = 1;
		bd.Data = Texture.data();

		auto glTexture = std::make_shared<AT2::GlTexture2D>(gli::internal_format(Texture.format()), gli::external_format(Texture.format()));
		glTexture->UpdateData(GL_TEXTURE_2D, bd);
		return glTexture;

	}
}

std::shared_ptr<AT2::GlVertexArray> MakeTerrainVAO()
{
	const int segX = 64, segY = 64;

	glm::vec2 texCoords [segX * segY * 4];
	
	for (int j = 0; j < segY; ++j)
	{
		for (int i = 0; i < segX; ++i)
		{
			const int num = (i + j * segX)*4;
			texCoords[num] = glm::vec2(float(i)/segX, float(j)/segY);
			texCoords[num+1] = glm::vec2(float(i+1)/segX, float(j)/segY);
			texCoords[num+2] = glm::vec2(float(i+1)/segX, float(j+1)/segY);
			texCoords[num+3] = glm::vec2(float(i)/segX, float(j+1)/segY);
		}
	}
	
	auto vao = std::make_shared<AT2::GlVertexArray>();
	vao->SetVertexBuffer(1, std::make_shared<AT2::GlVertexBuffer<glm::vec2>>(AT2::GlVertexBufferBase::GlBufferType::ArrayBuffer, segX * segY * 4, texCoords));
	return vao;
}

void Render(AT2::GlRenderer* renderer)
{
	//matMW = glm::lookAt(glm::vec3(-1.0,0.0,0.0), glm::vec3(0.0,0.0,0.0), glm::vec3(0.0, 1.0, 0.0));


	renderer->ClearBuffer(glm::vec4(0.0, 0.0, 1.0, 1.0));

	AT2::TextureSet cloudsTS;
	cloudsTS.insert(Noise3Tex);
	cloudsTS.insert(HeightMapTex);
	cloudsTS.insert(NormalMapTex);
	cloudsTS.insert(RockTex);
	cloudsTS.insert(GrassTex);
	renderer->GetStateManager()->BindTextures(cloudsTS);

	Shader->SetUniform("u_matMW", matMW);
	Shader->SetUniform("u_matInverseMW", glm::inverse(matMW));
	Shader->SetUniform("u_matProj", matProj);
	Shader->SetUniform("u_matInverseProj", glm::inverse(matProj));
	Shader->SetUniform("u_phase", Phase);
	Shader->SetUniform("u_texNoise", Noise3Tex->GetCurrentModule());
	Shader->SetUniform("u_texHeight", HeightMapTex->GetCurrentModule());
	
	TerrainShader->SetUniform("u_matMW", matMW);
	TerrainShader->SetUniform("u_matInverseMW", glm::inverse(matMW));
	TerrainShader->SetUniform("u_matProj", matProj);
	TerrainShader->SetUniform("u_matInverseProj", glm::inverse(matProj));
	TerrainShader->SetUniform("u_phase", Phase);
	TerrainShader->SetUniform("u_scaleH", 10000.0f);
	TerrainShader->SetUniform("u_scaleV", 1000.0f);
	TerrainShader->SetUniform("u_texHeight", HeightMapTex->GetCurrentModule());
	TerrainShader->SetUniform("u_texNormalMap", NormalMapTex->GetCurrentModule());
	TerrainShader->SetUniform("u_texGrass", GrassTex->GetCurrentModule());
	TerrainShader->SetUniform("u_texRock", RockTex->GetCurrentModule());


	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	renderer->GetStateManager()->BindShader(TerrainShader);
	renderer->GetStateManager()->BindVertexArray(TerrainVertexArray);
	glPatchParameteri( GL_PATCH_VERTICES, 4 );
	glDrawArrays(GL_PATCHES, 0, 64*64*4);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	renderer->GetStateManager()->BindShader(Shader);
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
			LoadShader("resources\\shaders\\raytrace_sky.vs.glsl"),
			"",
			"",
			"",
			LoadShader("resources\\shaders\\raytrace_sky.fs.glsl"));

		TerrainShader = std::make_shared<AT2::GlShaderProgram>(
			LoadShader("resources\\shaders\\terrain.vs.glsl"),
			LoadShader("resources\\shaders\\terrain.tcs.glsl"),
			LoadShader("resources\\shaders\\terrain.tes.glsl"),
			"",
			LoadShader("resources\\shaders\\terrain.fs.glsl"));

		auto texture = new AT2::GlTexture3D(GL_RGBA, GL_RGBA);
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

		GrassTex = LoadTexture("resources\\grass03.dds");
		RockTex = LoadTexture("resources\\rock04.dds");
		NormalMapTex = LoadTexture("resources\\terrain_normalmap.dds");
		HeightMapTex = LoadTexture("resources\\heightmap.dds");
		TerrainVertexArray = MakeTerrainVAO();


		glm::vec3 positions[] = {glm::vec3(-1.0, -1.0, -1.0), glm::vec3(1.0, -1.0, -1.0), glm::vec3(1.0, 1.0, -1.0), glm::vec3(-1.0, 1.0, -1.0)};
		GLuint indices[] = {0, 1, 2, 0, 2, 3};


		VertexArray = std::make_shared<AT2::GlVertexArray>();
		VertexArray->SetVertexBuffer(1, std::make_shared<AT2::GlVertexBuffer<glm::vec3>>(AT2::GlVertexBufferBase::GlBufferType::ArrayBuffer, 4, positions));
		VertexArray->SetIndexBuffer(std::make_shared<AT2::GlVertexBuffer<GLuint>>(AT2::GlVertexBufferBase::GlBufferType::ElementArrayBuffer, 6, indices));
		
		std::make_shared<AT2::GlUniformBuffer>(*TerrainShader, "MyBlock");

		//Init
		glEnable(GL_TEXTURE_1D);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_TEXTURE_3D);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glViewport(0, 0, 1024, 1024);
		matProj = glm::frustum(1.0, -1.0, -1.0, 1.0, 0.5, 5000.0);

		bool wireframe = false;
		float heading = 0.0f, pitch = 0.0f;
		glm::vec3 position = glm::vec3(0.0, 0.0, 0.0), direction;

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
							heading += sdlEvent.motion.xrel * 0.01f;
							pitch += sdlEvent.motion.yrel * 0.01f;
							pitch = glm::clamp(pitch, -glm::pi<float>()/2, glm::pi<float>()/2);

							direction = glm::vec3(cos(pitch) * sin(heading), sin(pitch), cos(pitch) * cos(heading));
						} break;
					case SDL_KEYDOWN:
						{
							if (sdlEvent.key.keysym.scancode == SDL_SCANCODE_Z && sdlEvent.key.state == SDL_PRESSED)
								wireframe = !wireframe;
							glPolygonMode(GL_FRONT_AND_BACK, (wireframe) ? GL_LINE : GL_FILL);
						} break;
					case SDL_QUIT:
						goto QuitLabel;
				}

				glm::vec3 right(sin(heading - 3.14f/2.0f), 0, cos(heading - 3.14f/2.0f));
				glm::vec3 up = glm::cross( right, direction);

				const Uint8* keyboardState = SDL_GetKeyboardState(NULL);
				if (keyboardState[SDL_SCANCODE_W])
					position += direction;
				if (keyboardState[SDL_SCANCODE_S])
					position -= direction;
				if (keyboardState[SDL_SCANCODE_A])
					position += right;
				if (keyboardState[SDL_SCANCODE_D])
					position -= right;

				matMW = glm::lookAt(position, position+direction, up);
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