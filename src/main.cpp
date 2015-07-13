//This file is something like sandbox. It is just functionality test, not example.

#include "OpenGl/GlRenderer.h"
#include "OpenGl/GlShaderProgram.h"
#include "OpenGl/GlUniformBuffer.h"
#include "OpenGl/GlTexture.h"
#include "OpenGl/GlVertexArray.h"
#include "OpenGl/GlFrameBuffer.h"
#include "OpenGl/GlUniformContainer.h"

#include "drawable.h"
#include "OpenGl/GlDrawPrimitive.h"

#include <iostream>
#include <fstream>
#include <SDL.h>

#include <glm/gtc/random.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <chrono>
#include <filesystem>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

std::shared_ptr<AT2::GlShaderProgram> MeshShader;

std::shared_ptr<AT2::GlUniformBuffer> CameraUB, LightUB;
std::shared_ptr<AT2::ITexture> Noise3Tex, HeightMapTex, NormalMapTex, RockTex, GrassTex;

std::shared_ptr<AT2::GlFrameBuffer> Stage1FBO, Stage2FBO;
std::shared_ptr<AT2::IFrameBuffer> NullFBO;

std::shared_ptr<AT2::MeshDrawable> QuadDrawable, TerrainDrawable, SphereLightDrawable;
std::vector<std::shared_ptr<AT2::IDrawable>> SceneDrawables;

bool WireframeMode = false;

GLfloat Phase = 0.0;

glm::mat4 matMV, matProj;

namespace AT2
{

class GlShaderProgramFromFile : public GlShaderProgram, public virtual IReloadable
{
public:
	GlShaderProgramFromFile(const str& vsFilename, const str& tcsFilename, const str& tesFilename, const str& gsFilename, const str& fsFilename) : GlShaderProgram()
	{
		m_filenames[0] = vsFilename;
		m_filenames[1] = tcsFilename;
		m_filenames[2] = tesFilename;
		m_filenames[3] = gsFilename;
		m_filenames[4] = fsFilename;

		SetName(vsFilename);
		Reload();
	}

	void Reload()
	{
		GlShaderProgram::Reload(
			LoadShader(m_filenames[0]),
			LoadShader(m_filenames[1]),
			LoadShader(m_filenames[2]),
			LoadShader(m_filenames[3]),
			LoadShader(m_filenames[4]));
	}

	~GlShaderProgramFromFile()
	{
	}

private:
	std::string LoadShader(const str& _filename)
	{
		if (_filename.empty())
			return "";

		std::ifstream t(_filename);
		return std::string((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
	}

private:
	str m_filenames[5];

private: //static
	static std::map<unsigned int, std::weak_ptr<GlShaderProgramFromFile>> s_allShaderPrograms;
	
public: //static
	static std::shared_ptr<GlShaderProgramFromFile> CreateShader(const str& vsFilename, const str& tcsFilename, const str& tesFilename, const str& gsFilename, const str& fsFilename)
	{
		auto shader = std::make_shared<AT2::GlShaderProgramFromFile>(vsFilename, tcsFilename, tesFilename, gsFilename, fsFilename);
		s_allShaderPrograms[shader->GetId()] = shader;

		return shader;
	}
	static void ReloadAll()
	{
		for (auto element : s_allShaderPrograms)
		{
			element.second.lock()->Reload();
		}
	}
};
std::map<unsigned int, std::weak_ptr<GlShaderProgramFromFile>> GlShaderProgramFromFile::s_allShaderPrograms;

}

std::shared_ptr<AT2::MeshDrawable> MakeSphereDrawable(AT2::GlRenderer* renderer, int segX = 32, int segY = 16)
{
	std::vector<glm::vec3> normals; normals.reserve(segX * segY);
	std::vector<GLuint> indices; indices.reserve(segX * segY * 6);

	for (int j = 0; j < segY; ++j)
	{
		double angV = j*M_PI / (segY-1);
		for (int i = 0; i < segX; ++i)
		{
			double angH = i*M_PI * 2 / segX;

			normals.push_back(glm::vec3(sin(angV)*cos(angH), sin(angV)*sin(angH), cos(angV)));
		}
	}

	for (int j = 0; j < segY-1; ++j)
	{
		const int nj = j + 1;
		for (int i = 0; i < segX; ++i)
		{
			const int ni = (i + 1) % segX;

			indices.push_back(j * segX + i);
			indices.push_back(j * segX + ni);
			indices.push_back(nj * segX + ni);
			indices.push_back(j * segX + i);
			indices.push_back(nj * segX + ni);
			indices.push_back(nj * segX + i);
		}
	}

	auto vao = std::make_shared<AT2::GlVertexArray>(renderer->GetRendererCapabilities());
	vao->SetVertexBuffer(1, std::make_shared<AT2::GlVertexBuffer<glm::vec3>>(AT2::GlVertexBufferBase::GlBufferType::ArrayBuffer, normals.size(), normals.data()));
	vao->SetIndexBuffer(std::make_shared<AT2::GlVertexBuffer<GLuint>>(AT2::GlVertexBufferBase::GlBufferType::ElementArrayBuffer, indices.size(), indices.data()));

	auto drawable = std::make_shared<AT2::MeshDrawable>();
	drawable->Primitives.push_back(new AT2::GlDrawElementsPrimitive(AT2::GlDrawPrimitiveType::Triangles, indices.size(), AT2::GlDrawElementsPrimitive::IndicesType::UnsignedInt, 0));
	drawable->VertexArray = vao;

	return drawable;
}

std::shared_ptr<AT2::MeshDrawable> MakeTerrainDrawable(AT2::GlRenderer* renderer, int segX, int segY)
{
	std::vector<glm::vec2> texCoords(segX * segY * 4);//TODO! GlVertexBuffer - take iterators!

	for (int j = 0; j < segY; ++j)
	{
		for (int i = 0; i < segX; ++i)
		{
			const int num = (i + j * segX) * 4;
			texCoords[num] = glm::vec2(float(i) / segX, float(j) / segY);
			texCoords[num + 1] = glm::vec2(float(i + 1) / segX, float(j) / segY);
			texCoords[num + 2] = glm::vec2(float(i + 1) / segX, float(j + 1) / segY);
			texCoords[num + 3] = glm::vec2(float(i) / segX, float(j + 1) / segY);
		}
	}

	auto vao = std::make_shared<AT2::GlVertexArray>(renderer->GetRendererCapabilities());
	vao->SetVertexBuffer(1, std::make_shared<AT2::GlVertexBuffer<glm::vec2>>(AT2::GlVertexBufferBase::GlBufferType::ArrayBuffer, texCoords.size(), texCoords.data()));


	auto drawable = std::make_shared<AT2::MeshDrawable>();
	drawable->Primitives.push_back(new AT2::GlDrawArraysPrimitive(AT2::GlDrawPrimitiveType::Patches, 0, texCoords.size()));
	drawable->VertexArray = vao;

	return drawable;
}

std::shared_ptr<AT2::MeshDrawable> MakeFullscreenQuadDrawable(AT2::GlRenderer* renderer)
{
	glm::vec3 positions[] = { glm::vec3(-1.0, -1.0, -1.0), glm::vec3(1.0, -1.0, -1.0), glm::vec3(1.0, 1.0, -1.0), glm::vec3(-1.0, 1.0, -1.0) };
	GLuint indices[] = { 0, 1, 2, 0, 2, 3 };

	auto vao = std::make_shared<AT2::GlVertexArray>(renderer->GetRendererCapabilities());
	vao->SetVertexBuffer(1, std::make_shared<AT2::GlVertexBuffer<glm::vec3>>(AT2::GlVertexBufferBase::GlBufferType::ArrayBuffer, 4, positions));
	vao->SetIndexBuffer(std::make_shared<AT2::GlVertexBuffer<GLuint>>(AT2::GlVertexBufferBase::GlBufferType::ElementArrayBuffer, 6, indices));

	auto drawable = std::make_shared<AT2::MeshDrawable>();
	drawable->Primitives.push_back(new AT2::GlDrawElementsPrimitive(AT2::GlDrawPrimitiveType::Triangles, 6, AT2::GlDrawElementsPrimitive::IndicesType::UnsignedInt, 0));
	drawable->VertexArray = vao;

	return drawable;
}

std::shared_ptr<AT2::IDrawable> LoadModel(const AT2::str& _filename, AT2::GlRenderer* _renderer)
{
	class GlMeshDrawable : public AT2::IDrawable
	{
	friend class GlMeshBuilder;

	public:
		GlMeshDrawable()
		{
			auto storage = std::make_shared<AT2::GlUniformContainer>(MeshShader);
		}

		void Draw(AT2::IRenderer& _renderer) override
		{
			auto stateManager = _renderer.GetStateManager();

			stateManager->BindShader(Shader);
			stateManager->BindVertexArray(VertexArray);
			UniformBuffer->Bind();

			for (auto& primitive : Primitives)
			{
				stateManager->BindTextures(primitive.Textures);

				UniformBuffer->SetUniform("u_matModel", primitive.ModelMatrix);

				primitive.Primitive->Draw();
			}
		}

	protected:
		std::shared_ptr<AT2::IShaderProgram> Shader;
		std::shared_ptr<AT2::IVertexArray> VertexArray;
		std::shared_ptr<AT2::IUniformContainer> UniformBuffer;

		struct ParametrizedPrimitive
		{
			glm::mat4 ModelMatrix;
			AT2::TextureSet Textures;
			std::shared_ptr<AT2::IDrawPrimitive> Primitive;
		};
		std::vector<ParametrizedPrimitive> Primitives;
	};

	class GlMeshBuilder
	{
	public:
		GlMeshBuilder(const AT2::IRenderer* _renderer, const AT2::str& _filename) : m_renderer(_renderer), m_scenePath(_filename)
		{
			m_scene = m_importer.ReadFile(_filename, aiProcess_Triangulate | aiProcess_ValidateDataStructure | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);
		}

	public:
		std::shared_ptr <GlMeshDrawable> m_buildedMesh;

		void Build()
		{
			BuildVAO();
			TraverseNode(m_scene->mRootNode, aiMatrix4x4());
		}

	protected:
		std::tr2::sys::path m_scenePath;
		Assimp::Importer m_importer;
		const aiScene* m_scene;
		const AT2::IRenderer* m_renderer;

		std::vector<glm::vec3> m_verticesVec;
		std::vector<glm::vec3> m_texCoordVec;
		std::vector<glm::vec3> m_normalsVec;
		std::vector<GLuint> m_indicesVec;

		std::vector<GLint> m_meshIndexOffsets;

	protected:
		void AddMesh(const aiMesh* _mesh)
		{
			auto vertexOffset = m_verticesVec.size();

			m_meshIndexOffsets.push_back(m_indicesVec.size());

			m_verticesVec.insert(m_verticesVec.end(), reinterpret_cast<glm::vec3*>(_mesh->mVertices), reinterpret_cast<glm::vec3*>(_mesh->mVertices) + _mesh->mNumVertices);
			m_texCoordVec.insert(m_texCoordVec.end(), reinterpret_cast<glm::vec3*>(_mesh->mTextureCoords[0]), reinterpret_cast<glm::vec3*>(_mesh->mTextureCoords[0]) + _mesh->mNumVertices);
			m_normalsVec.insert(m_normalsVec.end(), reinterpret_cast<glm::vec3*>(_mesh->mNormals), reinterpret_cast<glm::vec3*>(_mesh->mNormals) + _mesh->mNumVertices);

			for (int j = 0; j < _mesh->mNumFaces; ++j)
			{
				const aiFace& face = _mesh->mFaces[j];

				m_indicesVec.push_back(face.mIndices[0] + vertexOffset);
				m_indicesVec.push_back(face.mIndices[2] + vertexOffset);
				m_indicesVec.push_back(face.mIndices[1] + vertexOffset);
			}
		}

		void BuildVAO()
		{
			for (int i = 0; i < m_scene->mNumMeshes; ++i)
			{
				AddMesh(m_scene->mMeshes[i]);
			}

			auto vao = std::make_shared<AT2::GlVertexArray>(m_renderer->GetRendererCapabilities());
			vao->SetVertexBuffer(1, std::make_shared<AT2::GlVertexBuffer<glm::vec3>>(AT2::GlVertexBufferBase::GlBufferType::ArrayBuffer, m_verticesVec.size(), m_verticesVec.data()));
			vao->SetVertexBuffer(2, std::make_shared<AT2::GlVertexBuffer<glm::vec3>>(AT2::GlVertexBufferBase::GlBufferType::ArrayBuffer, m_texCoordVec.size(), m_texCoordVec.data()));
			vao->SetVertexBuffer(3, std::make_shared<AT2::GlVertexBuffer<glm::vec3>>(AT2::GlVertexBufferBase::GlBufferType::ArrayBuffer, m_normalsVec.size(), m_normalsVec.data()));
			vao->SetIndexBuffer(std::make_shared<AT2::GlVertexBuffer<GLuint>>(AT2::GlVertexBufferBase::GlBufferType::ElementArrayBuffer, m_indicesVec.size(), m_indicesVec.data()));

			m_buildedMesh = std::make_shared<GlMeshDrawable>();
			m_buildedMesh->Shader = MeshShader;
			m_buildedMesh->VertexArray = vao;
		}

		void ExtractMaterials()
		{
			for (int i = 0; i < m_scene->mNumMaterials; ++i)
			{
				const aiMaterial* material = m_scene->mMaterials[i];

				aiString path;
				if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
				{
					std::tr2::sys::path modelPath = m_scenePath.parent_path();
					modelPath /= path.data;

					auto texDiffuse = m_renderer->GetResourceFactory()->LoadTexture(modelPath.string());
					//pp.Textures.insert(texDiffuse);
					//storage->SetUniform("u_texDiffuse", texDiffuse);
				}
			}
		}

		void TraverseNode(const aiNode* _node, aiMatrix4x4 _transform)
		{
			if (_node->mNumMeshes)
			{
				for (int i = 0; i < _node->mNumMeshes; i++)
				{
					const int meshIndex = _node->mMeshes[i];
					const aiMesh* mesh = m_scene->mMeshes[meshIndex];

					GlMeshDrawable::ParametrizedPrimitive pp;
					pp.Primitive = std::make_shared<AT2::GlDrawElementsPrimitive>(AT2::GlDrawPrimitiveType::Triangles, mesh->mNumFaces * 3, AT2::GlDrawElementsPrimitive::IndicesType::UnsignedInt, m_meshIndexOffsets[meshIndex]*sizeof(GLuint));
					pp.ModelMatrix = ConvertMatrix(_transform);
					
					m_buildedMesh->Primitives.push_back(pp);
					if (!m_buildedMesh->UniformBuffer)
						m_buildedMesh->UniformBuffer = std::make_shared<AT2::GlUniformContainer>(MeshShader);
				}
			}
			else
			{
				_transform = _node->mTransformation * _transform;
			}

			for (int i = 0; i < _node->mNumChildren; i++)
				TraverseNode(_node->mChildren[i], _transform);
		}

		glm::mat4 ConvertMatrix(const aiMatrix4x4& _t) const
		{
			return glm::transpose(*(reinterpret_cast<const glm::mat4*>(&_t)));
		}
		
	};

	GlMeshBuilder meshBuilder(_renderer, _filename);
	meshBuilder.Build();
	return meshBuilder.m_buildedMesh;
}

class GlTimerQuery
{
public:
	GlTimerQuery()
	{
		glGenQueries(1, &m_id);
	}
	~GlTimerQuery()
	{
		glDeleteQueries(1, &m_id);
	}

	void Begin()
	{
		m_resultValue = 0;
		glBeginQuery(GL_TIME_ELAPSED, m_id);
	}

	void End()
	{
		glEndQuery(GL_TIME_ELAPSED);
	}

	GLuint64 WaitForResult()
	{
		glGetQueryObjectui64v(m_id, GL_QUERY_RESULT, &m_resultValue);
		return m_resultValue;
	}

private:
	GLuint m_id;
	GLuint64 m_resultValue;
};

std::vector<std::shared_ptr<AT2::GlUniformBuffer>> LightsArray;

//returns frame time
float Render(AT2::GlRenderer* renderer)
{
	auto timeBefore = std::chrono::high_resolution_clock::now();

	GlTimerQuery glTimer;
	glTimer.Begin();

	CameraUB->SetUniform("u_matView", matMV);
	CameraUB->SetUniform("u_matInverseView", glm::inverse(matMV));
	CameraUB->SetUniform("u_matProjection", matProj);
	CameraUB->SetUniform("u_matInverseProjection", glm::inverse(matProj));
	CameraUB->SetUniform("u_matViewProjection", matProj * matMV);
	CameraUB->SetUniform("u_matNormal", glm::transpose(glm::inverse(glm::mat3(matMV))));
	CameraUB->Bind();
	

	//Scene stage
	Stage1FBO->Bind();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	renderer->ClearBuffer(glm::vec4(0.0, 0.0, 1.0, 1.0));
	renderer->ClearDepth(1.0);

	glPolygonMode(GL_FRONT_AND_BACK, (WireframeMode) ? GL_LINE : GL_FILL);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
	glCullFace(GL_BACK);
	glPatchParameteri( GL_PATCH_VERTICES, 4 );
	TerrainDrawable->Draw(*renderer);

	for (const auto& drawable : SceneDrawables)
	{
		drawable->Draw(*renderer);
	}

	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	//Light stage
	Stage2FBO->Bind();

	renderer->ClearBuffer(glm::vec4(0.0, 0.0, 0.0, 0.0));
	//glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_GREATER);
	glCullFace(GL_FRONT);

	for (const auto& light : LightsArray)
	{
		light->SetBindingPoint(2);
		light->Bind();
		SphereLightDrawable->Draw(*renderer);
	}

	
	//Postprocess stage
	NullFBO->Bind();
	glDepthMask(GL_TRUE);
	renderer->ClearBuffer(glm::vec4(0.0, 0.0, 0.0, 0.0));

	glCullFace(GL_BACK);
	glDisable(GL_DEPTH_TEST);
	(dynamic_cast<AT2::GlShaderProgram*>(QuadDrawable->Shader.get()))->SetUBO("CameraBlock", 1);
	QuadDrawable->Draw(*renderer);




	glTimer.End();
	float frameTime = glTimer.WaitForResult() * 0.000001; //in ms

	glFinish();
	renderer->SwapBuffers();


	//float frameTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - timeBefore).count() * 0.001;
	return frameTime;
}

int main(int argc, char *argv[])
{
	try
	{
		auto renderer = new AT2::GlRenderer();


		auto postprocessShader = AT2::GlShaderProgramFromFile::CreateShader(
			"resources\\shaders\\postprocess.vs.glsl",
			"",
			"",
			"",
			"resources\\shaders\\postprocess.fs.glsl");

		auto terrainShader = AT2::GlShaderProgramFromFile::CreateShader(
			"resources\\shaders\\terrain.vs.glsl",
			"resources\\shaders\\terrain.tcs.glsl",
			"resources\\shaders\\terrain.tes.glsl",
			"",
			"resources\\shaders\\terrain.fs.glsl");

		auto sphereLightShader = AT2::GlShaderProgramFromFile::CreateShader(
			"resources\\shaders\\spherelight.vs.glsl",
			"",
			"",
			"",
			"resources\\shaders\\spherelight.fs.glsl");

		auto dayLightShader = AT2::GlShaderProgramFromFile::CreateShader(
			"resources\\shaders\\skylight.vs.glsl",
			"",
			"",
			"",
			"resources\\shaders\\skylight.fs.glsl");

		MeshShader = AT2::GlShaderProgramFromFile::CreateShader(
			"resources\\shaders\\mesh.vs.glsl",
			"",
			"",
			"",
			"resources\\shaders\\mesh.fs.glsl");

		auto texture = new AT2::GlTexture3D(GL_RGBA8, glm::uvec3(256, 256, 256), 1);
		AT2::GlTexture::BufferData data;
		data.Height = 256;
		data.Width = 256;
		data.Depth = 256;
		GLubyte* arr = new GLubyte [data.Height * data.Width * data.Depth * 4];
		for (int i = 0; i < data.Height * data.Width * data.Depth * 4; ++i)
		   arr[i] = (rand() & 0xFF);
		data.Data = arr;
		texture->SetData(0, data);
		delete [] arr;

		Noise3Tex = std::shared_ptr<AT2::ITexture>(texture);

		GrassTex = renderer->GetResourceFactory()->LoadTexture("resources\\grass03.dds");
		RockTex = renderer->GetResourceFactory()->LoadTexture("resources\\rock04.dds");
		NormalMapTex = renderer->GetResourceFactory()->LoadTexture("resources\\terrain_normalmap.dds");
		HeightMapTex = renderer->GetResourceFactory()->LoadTexture("resources\\heightmap.dds");


		CameraUB = std::make_shared<AT2::GlUniformBuffer>(terrainShader->GetUniformBlockInfo("CameraBlock"));
		CameraUB->SetBindingPoint(1);
		LightUB = std::make_shared<AT2::GlUniformBuffer>(sphereLightShader->GetUniformBlockInfo("LightingBlock"));

		auto texDiffuseRT = std::make_shared<AT2::GlTexture2D>(GL_RGBA8, glm::uvec2(1024, 1024));
		auto texNormalRT = std::make_shared<AT2::GlTexture2D>(GL_RGBA32F, glm::uvec2(1024, 1024));
		auto texDepthRT = std::make_shared<AT2::GlTexture2D>(GL_DEPTH_COMPONENT32F, glm::uvec2(1024, 1024));

		Stage1FBO = std::make_shared<AT2::GlFrameBuffer>(renderer->GetRendererCapabilities());
		Stage1FBO->SetColorAttachement(0, texDiffuseRT);
		Stage1FBO->SetColorAttachement(1, texNormalRT);
		Stage1FBO->SetDepthAttachement(texDepthRT);

		auto texColorRT = std::make_shared<AT2::GlTexture2D>(GL_RGBA32F, glm::uvec2(1024, 1024));

		Stage2FBO = std::make_shared<AT2::GlFrameBuffer>(renderer->GetRendererCapabilities());
		Stage2FBO->SetColorAttachement(0, texColorRT);
		Stage2FBO->SetDepthAttachement(texDepthRT); //depth is common with previous stage

		NullFBO = std::make_shared<AT2::GlScreenFrameBuffer>();

		//terrain
		TerrainDrawable = MakeTerrainDrawable(renderer, 64, 64);
		TerrainDrawable->Shader = terrainShader;
		TerrainDrawable->Textures = { Noise3Tex, HeightMapTex, NormalMapTex, RockTex, GrassTex };
		{
			auto uniformStorage = std::make_shared<AT2::GlUniformContainer>(terrainShader);
			uniformStorage->SetUniform("u_phase", Phase);
			uniformStorage->SetUniform("u_scaleH", 10000.0f);
			uniformStorage->SetUniform("u_scaleV", 800.0f);
			uniformStorage->SetUniform("u_texHeight", HeightMapTex);
			uniformStorage->SetUniform("u_texNormalMap", NormalMapTex);
			uniformStorage->SetUniform("u_texGrass", GrassTex);
			uniformStorage->SetUniform("u_texRock", RockTex);
			TerrainDrawable->UniformBuffer = uniformStorage;
		}

		SphereLightDrawable = MakeSphereDrawable(renderer);
		SphereLightDrawable->Shader = sphereLightShader;
		SphereLightDrawable->Textures = { Stage1FBO->GetColorAttachement(0), Stage1FBO->GetColorAttachement(1), Stage1FBO->GetDepthAttachement(), Noise3Tex };
		{
			auto uniformStorage = std::make_shared<AT2::GlUniformContainer>(sphereLightShader);
			uniformStorage->SetUniform("u_texNoise", Noise3Tex);
			uniformStorage->SetUniform("u_colorMap", Stage1FBO->GetColorAttachement(0));
			uniformStorage->SetUniform("u_normalMap", Stage1FBO->GetColorAttachement(1));
			uniformStorage->SetUniform("u_depthMap", Stage1FBO->GetDepthAttachement());
			
			SphereLightDrawable->UniformBuffer = uniformStorage;
		}

		//Postprocess quad
		QuadDrawable = MakeFullscreenQuadDrawable(renderer);
		QuadDrawable->Shader = postprocessShader;
		QuadDrawable->Textures = { Stage2FBO->GetColorAttachement(0), Stage2FBO->GetDepthAttachement(), Noise3Tex, Stage1FBO->GetColorAttachement(0), GrassTex };
		{
			auto uniformStorage = std::make_shared<AT2::GlUniformContainer>(postprocessShader);
			uniformStorage->SetUniform("u_phase", Phase);
			uniformStorage->SetUniform("u_texNoise", Noise3Tex);
			uniformStorage->SetUniform("u_colorMap", Stage2FBO->GetColorAttachement(0));
			uniformStorage->SetUniform("u_depthMap", Stage2FBO->GetDepthAttachement());
			QuadDrawable->UniformBuffer = uniformStorage;
		}


		for (int i = 0; i < 100; ++i)
		{
			auto light = std::make_shared<AT2::GlUniformBuffer>(sphereLightShader->GetUniformBlockInfo("LightingBlock"));
			
			light->SetUniform("u_lightPos", glm::vec4(glm::linearRand(-5000.0, 5000.0), glm::linearRand(-300.0, 100.0), glm::linearRand(-5000.0, 5000.0), 1.0));
			light->SetUniform("u_lightRadius", glm::linearRand(300.0f, 700.0f)*2.0f);
			light->SetUniform("u_lightColor", glm::linearRand(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f)));
			light->SetBindingPoint(2);
			LightsArray.push_back(light);
		}

		SceneDrawables.push_back(LoadModel("resources/jeep1.3ds", renderer));

		//Init
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_CULL_FACE);

		glViewport(0, 0, 1024, 1024);
		matProj = glm::perspective(90.0, -1.0, 1.0, 10000.0);//

		float heading = 0.0f, pitch = 0.0f;
		glm::vec3 position = glm::vec3(0.0, 0.0, 0.0), direction;

		Uint32 startTime = SDL_GetTicks(); 
		Uint32 fpsCount = 0;

		double framesSummaryTime = 0.0;
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

							direction = glm::normalize(glm::vec3(cos(pitch) * sin(heading), sin(pitch), cos(pitch) * cos(heading)));
						} break;
					case SDL_KEYDOWN:
						{
							if (sdlEvent.key.keysym.scancode == SDL_SCANCODE_Z && sdlEvent.key.state == SDL_PRESSED)
								WireframeMode = !WireframeMode;
						} break;
					case SDL_QUIT:
						goto QuitLabel;
					case SDL_WINDOWEVENT:
					{
						if (sdlEvent.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
						{
							//AT2::GlShaderProgramFromFile::ReloadAll();
						}
					} break;
				}
			}
			glm::vec3 right(sin(heading - 3.14f / 2.0f), 0, cos(heading - 3.14f / 2.0f));
			glm::vec3 up = glm::cross(right, direction);

			const Uint8* keyboardState = SDL_GetKeyboardState(NULL);
			if (keyboardState[SDL_SCANCODE_W])
				position += direction;
			if (keyboardState[SDL_SCANCODE_S])
				position -= direction;
			if (keyboardState[SDL_SCANCODE_A])
				position += right;
			if (keyboardState[SDL_SCANCODE_D])
				position -= right;

			matMV = glm::lookAt(position, position + direction, up);

			LightsArray[0]->SetUniform("u_lightPos", glm::vec4(position, 1.0));

			Uint32 time = SDL_GetTicks();
			if (time - startTime > 1000)
			{
				//std::cout << fpsCount << " ";
				std::cout << framesSummaryTime/fpsCount << "ms ";


				startTime = time;
				fpsCount = 0;
				framesSummaryTime = 0.0;
			}
			fpsCount++;

			framesSummaryTime += Render(renderer);
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