//This file is something like sandbox. It is just functionality test, not example.

#include <AT2/OpenGL/GlRenderer.h>
#include <AT2/OpenGL/GlShaderProgram.h>
#include <AT2/OpenGL/GlUniformBuffer.h>
#include <AT2/OpenGL/GlTexture.h>
#include <AT2/OpenGL/GlVertexArray.h>
#include <AT2/OpenGL/GlFrameBuffer.h>
#include <AT2/OpenGL/GlUniformContainer.h>
#include <AT2/OpenGL/GlTimerQuery.h>
#include <AT2/OpenGL/GLFW/glfw_window.h>

#include "../drawable.h"
#include <AT2/OpenGL/GlDrawPrimitive.h>

#include <iostream>
#include <fstream>
#include <filesystem>

#include <glm/gtc/random.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <chrono>

#include "AT2/OpenGL/GLFW/glfw_application.h"

#define USE_ASSIMP

#ifdef USE_ASSIMP
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#endif


//#define AT2_USE_OCULUS_RIFT

#ifdef AT2_USE_OCULUS_RIFT
#include <Kernel/OVR_System.h>
#include <OVR_CAPI_GL.h>
#include <Extras/OVR_Math.h>
ovrHmd HMD;

#pragma comment (lib, "LibOVRKernel.lib")
#pragma comment (lib, "LibOVR.lib")
#endif

std::shared_ptr<AT2::IShaderProgram> MeshShader;

std::shared_ptr<AT2::GlUniformBuffer> CameraUB, LightUB;
std::shared_ptr<AT2::ITexture> Noise3Tex, HeightMapTex, NormalMapTex, RockTex, GrassTex, EnvironmentMapTex;

std::shared_ptr<AT2::GlFrameBuffer> Stage1FBO, Stage2FBO;
std::shared_ptr<AT2::IFrameBuffer> NullFBO;

std::shared_ptr<AT2::MeshDrawable> QuadDrawable, SkylightDrawable, TerrainDrawable, SphereLightDrawable;
std::vector<std::shared_ptr<AT2::IDrawable>> SceneDrawables;

bool WireframeMode = false, MovingLightMode = true;
size_t NumActiveLights = 50;

GLfloat Phase = 0.0;

#ifdef USE_ASSIMP

std::shared_ptr<AT2::IDrawable> LoadModel(const AT2::str& _filename, const std::shared_ptr<AT2::IRenderer>& _renderer)
{
	class GlMeshDrawable : public AT2::IDrawable
	{
	friend class GlMeshBuilder;

	public:
		GlMeshDrawable()
		{
			auto storage = MeshShader->CreateAssociatedUniformStorage();
		}

		void Draw(const std::shared_ptr<AT2::IRenderer>& _renderer) override
		{
			auto& stateManager = _renderer->GetStateManager();

			stateManager.BindShader(Shader);
			stateManager.BindVertexArray(VertexArray);
			UniformBuffer->Bind();

			for (auto& primitive : Primitives)
			{
				stateManager.BindTextures(primitive.Textures);

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
		GlMeshBuilder(const std::shared_ptr<AT2::IRenderer>& _renderer, const AT2::str& _filename) : m_renderer(_renderer), m_scenePath(_filename)
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
		std::filesystem::path m_scenePath;
		Assimp::Importer m_importer;
		const aiScene* m_scene;
		const std::shared_ptr<AT2::IRenderer>& m_renderer;

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

			auto& rf = m_renderer->GetResourceFactory();
			auto vao = rf.CreateVertexArray();
			vao->SetVertexBuffer(1, rf.CreateVertexBuffer(AT2vbt::ArrayBuffer, AT2::BufferDataTypes::Vec3, m_verticesVec.size(), m_verticesVec.data()));
			vao->SetVertexBuffer(2, rf.CreateVertexBuffer(AT2vbt::ArrayBuffer, AT2::BufferDataTypes::Vec3, m_texCoordVec.size(), m_texCoordVec.data()));
			vao->SetVertexBuffer(3, rf.CreateVertexBuffer(AT2vbt::ArrayBuffer, AT2::BufferDataTypes::Vec3, m_normalsVec.size(), m_normalsVec.data()));
			vao->SetIndexBuffer(rf.CreateVertexBuffer(AT2vbt::IndexBuffer, AT2::BufferDataTypes::UInt, m_indicesVec.size(), m_indicesVec.data()));

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
					std::filesystem::path modelPath = m_scenePath.parent_path();
					modelPath /= path.data;

					auto texDiffuse = m_renderer->GetResourceFactory().LoadTexture(modelPath.string());
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
						m_buildedMesh->UniformBuffer = MeshShader->CreateAssociatedUniformStorage();
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
#endif


std::vector<std::shared_ptr<AT2::GlUniformBuffer>> LightsArray;

//returns frame time
float Render(const std::shared_ptr<AT2::IRenderer>& renderer, AT2::IFrameBuffer* framebuffer, glm::mat4 matProj, glm::mat4 matMV)
{
	auto timeBefore = std::chrono::high_resolution_clock::now();

	AT2::GlTimerQuery glTimer;
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

	renderer->ClearBuffer(glm::vec4(0.0, 0.0, 1.0, 0.0));
	renderer->ClearDepth(1.0);

	glPolygonMode(GL_FRONT_AND_BACK, (WireframeMode) ? GL_LINE : GL_FILL);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
	glCullFace(GL_BACK);
	glPatchParameteri( GL_PATCH_VERTICES, 4 );
	TerrainDrawable->Draw(renderer);

	for (const auto& drawable : SceneDrawables)
	{
		drawable->Draw(renderer);
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
		SphereLightDrawable->Draw(renderer);
	}
	
	glCullFace(GL_BACK);
	glDisable(GL_DEPTH_TEST);
	SkylightDrawable->Draw(renderer);
	
	//Postprocess stage
	framebuffer->Bind();
	//glEnable(GL_FRAMEBUFFER_SRGB);

	glDepthMask(GL_TRUE);
	renderer->ClearBuffer(glm::vec4(0.0, 0.0, 0.0, 0.0));
	renderer->ClearDepth(0);

	glCullFace(GL_BACK);
	glDisable(GL_DEPTH_TEST);
	(dynamic_cast<AT2::GlShaderProgram*>(QuadDrawable->Shader.get()))->SetUBO("CameraBlock", 1);
	QuadDrawable->Draw(renderer);


	glTimer.End();
	float frameTime = glTimer.WaitForResult() * 0.000001; //in ms

	glFinish();

	return frameTime;
}

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
			setSize(m_framebufferPhysicalSize).
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
			throw new GlfwException("Failed to initialize GLEW"); //yes, it's strange to throw a Glfw exception :3

		m_renderer = std::make_unique<AT2::GlRenderer>();


		auto postprocessShader = m_renderer->GetResourceFactory().CreateShaderProgramFromFiles({
			"resources/shaders/postprocess.vs.glsl",
			"resources/shaders/postprocess.fs.glsl" });

		auto terrainShader = m_renderer->GetResourceFactory().CreateShaderProgramFromFiles({
			"resources/shaders/terrain.vs.glsl",
			"resources/shaders/terrain.tcs.glsl",
			"resources/shaders/terrain.tes.glsl",
			"resources/shaders/terrain.fs.glsl" });

		auto sphereLightShader = m_renderer->GetResourceFactory().CreateShaderProgramFromFiles({
			"resources/shaders/spherelight.vs.glsl",
			"resources/shaders/pbr.fs.glsl",
			"resources/shaders/spherelight.fs.glsl" });

		auto dayLightShader = m_renderer->GetResourceFactory().CreateShaderProgramFromFiles({
			"resources/shaders/skylight.vs.glsl",
			"resources/shaders/pbr.fs.glsl",
			"resources/shaders/skylight.fs.glsl" });

#ifdef USE_ASSIMP
		MeshShader = m_renderer->GetResourceFactory().CreateShaderProgramFromFiles({
			"resources/shaders/mesh.vs.glsl",
			"resources/shaders/mesh.fs.glsl" });
#endif
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

		GrassTex = m_renderer->GetResourceFactory().LoadTexture("resources/grass03.dds");
		RockTex = m_renderer->GetResourceFactory().LoadTexture("resources/rock04.dds");
		NormalMapTex = m_renderer->GetResourceFactory().LoadTexture("resources/terrain_normalmap.dds");
		HeightMapTex = m_renderer->GetResourceFactory().LoadTexture("resources/heightmap.dds");
		EnvironmentMapTex = m_renderer->GetResourceFactory().LoadTexture("resources/04-23_Day_D.hdr");

		CameraUB = std::make_shared<AT2::GlUniformBuffer>(std::dynamic_pointer_cast<AT2::GlShaderProgram>(terrainShader)->GetUniformBlockInfo("CameraBlock"));
		CameraUB->SetBindingPoint(1);
		LightUB = std::make_shared<AT2::GlUniformBuffer>(std::dynamic_pointer_cast<AT2::GlShaderProgram>(sphereLightShader)->GetUniformBlockInfo("LightingBlock"));

		auto texDiffuseRT = std::make_shared<AT2::GlTexture2D>(GL_RGBA8, glm::uvec2(1024, 1024));
		auto texNormalRT = std::make_shared<AT2::GlTexture2D>(GL_RGBA32F, glm::uvec2(1024, 1024));
		auto texDepthRT = std::make_shared<AT2::GlTexture2D>(GL_DEPTH_COMPONENT32F, glm::uvec2(1024, 1024));

		Stage1FBO = std::make_shared<AT2::GlFrameBuffer>(m_renderer->GetRendererCapabilities());
		Stage1FBO->SetColorAttachement(0, texDiffuseRT);
		Stage1FBO->SetColorAttachement(1, texNormalRT);
		Stage1FBO->SetDepthAttachement(texDepthRT);

		auto texColorRT = std::make_shared<AT2::GlTexture2D>(GL_RGBA32F, glm::uvec2(1024, 1024));

		Stage2FBO = std::make_shared<AT2::GlFrameBuffer>(m_renderer->GetRendererCapabilities());
		Stage2FBO->SetColorAttachement(0, texColorRT);
		Stage2FBO->SetDepthAttachement(texDepthRT); //depth is common with previous stage

		NullFBO = std::make_shared<AT2::GlScreenFrameBuffer>();

		//terrain
		TerrainDrawable = AT2::MeshDrawable::MakeTerrainDrawable(m_renderer, 64, 64);
		TerrainDrawable->Shader = terrainShader;
		TerrainDrawable->Textures = { Noise3Tex, HeightMapTex, NormalMapTex, RockTex, GrassTex };
		{
			auto uniformStorage = terrainShader->CreateAssociatedUniformStorage();
			uniformStorage->SetUniform("u_phase", Phase);
			uniformStorage->SetUniform("u_scaleH", 10000.0f);
			uniformStorage->SetUniform("u_scaleV", 800.0f);
			uniformStorage->SetUniform("u_texHeight", HeightMapTex);
			uniformStorage->SetUniform("u_texNormalMap", NormalMapTex);
			uniformStorage->SetUniform("u_texGrass", GrassTex);
			uniformStorage->SetUniform("u_texRock", RockTex);
			TerrainDrawable->UniformBuffer = uniformStorage;
		}

		SphereLightDrawable = AT2::MeshDrawable::MakeSphereDrawable(m_renderer);
		SphereLightDrawable->Shader = sphereLightShader;
		SphereLightDrawable->Textures = { Stage1FBO->GetColorAttachement(0), Stage1FBO->GetColorAttachement(1), Stage1FBO->GetDepthAttachement(), Noise3Tex };
		{
			auto uniformStorage = sphereLightShader->CreateAssociatedUniformStorage();
			uniformStorage->SetUniform("u_texNoise", Noise3Tex);
			uniformStorage->SetUniform("u_colorMap", Stage1FBO->GetColorAttachement(0));
			uniformStorage->SetUniform("u_normalMap", Stage1FBO->GetColorAttachement(1));
			uniformStorage->SetUniform("u_depthMap", Stage1FBO->GetDepthAttachement());

			SphereLightDrawable->UniformBuffer = uniformStorage;
		}


		SkylightDrawable = AT2::MeshDrawable::MakeFullscreenQuadDrawable(m_renderer);
		SkylightDrawable->Shader = dayLightShader;
		SkylightDrawable->Textures = { Stage1FBO->GetColorAttachement(0), Stage1FBO->GetColorAttachement(1), Stage1FBO->GetDepthAttachement(), Noise3Tex, EnvironmentMapTex };
		{
			auto uniformStorage = dayLightShader->CreateAssociatedUniformStorage();
			uniformStorage->SetUniform("u_phase", Phase);
			uniformStorage->SetUniform("u_texNoise", Noise3Tex);
			uniformStorage->SetUniform("u_colorMap", Stage1FBO->GetColorAttachement(0));
			uniformStorage->SetUniform("u_normalMap", Stage1FBO->GetColorAttachement(1));
			uniformStorage->SetUniform("u_depthMap", Stage1FBO->GetDepthAttachement());
			uniformStorage->SetUniform("u_environmentMap", EnvironmentMapTex);
			SkylightDrawable->UniformBuffer = uniformStorage;
		}

		//Postprocess quad
		QuadDrawable = AT2::MeshDrawable::MakeFullscreenQuadDrawable(m_renderer);
		QuadDrawable->Shader = postprocessShader;
		QuadDrawable->Textures = { Stage2FBO->GetColorAttachement(0), Stage2FBO->GetDepthAttachement(), Noise3Tex, Stage1FBO->GetColorAttachement(0), GrassTex };
		{
			auto uniformStorage = postprocessShader->CreateAssociatedUniformStorage();
			uniformStorage->SetUniform("u_phase", Phase);
			uniformStorage->SetUniform("u_texNoise", Noise3Tex);
			uniformStorage->SetUniform("u_colorMap", Stage2FBO->GetColorAttachement(0));
			uniformStorage->SetUniform("u_depthMap", Stage2FBO->GetDepthAttachement());
			QuadDrawable->UniformBuffer = uniformStorage;
		}


		for (int i = 0; i < NumActiveLights; ++i)
		{
			auto light = std::make_shared<AT2::GlUniformBuffer>(std::dynamic_pointer_cast<AT2::GlShaderProgram>(sphereLightShader)->GetUniformBlockInfo("LightingBlock"));

			light->SetUniform("u_lightPos", glm::vec4(glm::linearRand(-5000.0, 5000.0), glm::linearRand(-300.0, 100.0), glm::linearRand(-5000.0, 5000.0), 1.0));
			light->SetUniform("u_lightRadius", glm::linearRand(300.0f, 700.0f)*2.0f);
			light->SetUniform("u_lightColor", glm::linearRand(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f)));
			light->SetBindingPoint(2);
			LightsArray.push_back(light);
		}

		LightsArray[0]->SetUniform("u_lightColor", glm::vec3(1.0f, 0.0f, 0.5f));

#ifdef USE_ASSIMP
		SceneDrawables.push_back(LoadModel("resources/jeep1.3ds", m_renderer));
#endif


#ifdef AT2_USE_OCULUS_RIFT
		OVR::System::Init();
		// Initializes LibOVR, and the Rift
		if (!OVR_SUCCESS(ovr_Initialize(nullptr)))
			throw AT2::AT2Exception(AT2::AT2Exception::ErrorCase::Renderer, "Failed to initialize libOVR.");

		ovrGraphicsLuid luid;
		if (!OVR_SUCCESS(ovr_Create(&HMD, &luid)))
			throw AT2::AT2Exception(AT2::AT2Exception::ErrorCase::Renderer, "Failed to get HMD device.");

		ovrHmdDesc hmdDesc = ovr_GetHmdDesc(HMD);

		if (!OVR_SUCCESS(ovr_ConfigureTracking(HMD, ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection | ovrTrackingCap_Position, 0)))
			throw AT2::AT2Exception(AT2::AT2Exception::ErrorCase::Renderer, "Failed to configure tracking.");


		ovrEyeRenderDesc EyeRenderDesc[2];
		std::shared_ptr<AT2::GlFrameBuffer> eyeFrameBuffer[2];
		ovrSwapTextureSet* eyeTextureSet[2];

		std::shared_ptr<AT2::GlTexture2D> eyeRenderTextureDepth[2];
		std::vector<std::shared_ptr<AT2::GlTexture2D>> eyeRenderTextureList[2];
		for (int eye = 0; eye < 2; ++eye)
		{
			ovrSizei idealTextureSize = ovr_GetFovTextureSize(HMD, ovrEyeType(eye), hmdDesc.DefaultEyeFov[eye], 1);

			const GLuint format = GL_RGBA8; //GL_SRGB8_ALPHA8
			if (!OVR_SUCCESS(ovr_CreateSwapTextureSetGL(HMD, format, idealTextureSize.w, idealTextureSize.h, &eyeTextureSet[eye])))
				throw AT2::AT2Exception(AT2::AT2Exception::ErrorCase::Renderer, "Failed to create swap texture");

			eyeRenderTextureDepth[eye] = std::make_shared<AT2::GlTexture2D>(GL_DEPTH_COMPONENT32F, glm::uvec2(idealTextureSize.w, idealTextureSize.h));

			eyeFrameBuffer[eye] = std::make_shared<AT2::GlFrameBuffer>(renderer->GetRendererCapabilities());
			eyeFrameBuffer[eye]->SetDepthAttachement(eyeRenderTextureDepth[eye]);
			//eyeFrameBuffer[eye]->SetColorAttachement(0, std::make_shared<AT2::GlTexture2D>(GL_SRGB8_ALPHA8, glm::uvec2(idealTextureSize.w, idealTextureSize.h)));

			for (int i = 0; i < eyeTextureSet[eye]->TextureCount; ++i)
			{
				auto currentTexture = (ovrGLTexture*)&eyeTextureSet[eye]->Textures[i];
				eyeRenderTextureList[eye].push_back(std::make_shared<AT2::GlTexture2D>(currentTexture->OGL.TexId, format, glm::uvec2(currentTexture->Texture.Header.TextureSize.w, currentTexture->Texture.Header.TextureSize.h)));
			}
			eyeFrameBuffer[eye]->SetColorAttachement(0, eyeRenderTextureList[eye][0]);

			EyeRenderDesc[eye] = ovr_GetRenderDesc(HMD, ovrEyeType(eye), hmdDesc.DefaultEyeFov[eye]);
		}
		
		SDL_GL_SetSwapInterval(0);
#endif


		//Init
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_CULL_FACE);

		matProj = glm::perspective(glm::radians(90.0), -1.0, 1.0, 10000.0);//
	}

	void OnRender(double dt)
	{
		right = glm::vec3(sin(heading - 3.14f / 2.0f), 0, cos(heading - 3.14f / 2.0f));
		up = glm::cross(right, direction);
		matMV = glm::lookAt(position, position + direction, up);


		glViewport(0, 0, m_framebufferPhysicalSize.x, m_framebufferPhysicalSize.y);

		if (MovingLightMode)
			LightsArray[0]->SetUniform("u_lightPos", glm::vec4(position, 1.0));

#ifndef AT2_USE_OCULUS_RIFT
		Render(m_renderer, NullFBO.get(), matProj, matMV);
#else
		// Get eye poses, feeding in correct IPD offset
		ovrVector3f      ViewOffset[2] = { EyeRenderDesc[0].HmdToEyeViewOffset, EyeRenderDesc[1].HmdToEyeViewOffset };
		ovrPosef         EyeRenderPose[2];

		ovrFrameTiming   ftiming = ovr_GetFrameTiming(HMD, 0);
		ovrTrackingState hmdState = ovr_GetTrackingState(HMD, ftiming.DisplayMidpointSeconds);
		ovr_CalcEyePoses(hmdState.HeadPose.ThePose, ViewOffset, EyeRenderPose);

		//render
		for (int eye = 0; eye < 2; ++eye)
		{
			eyeTextureSet[eye]->CurrentIndex = (eyeTextureSet[eye]->CurrentIndex + 1) % eyeTextureSet[eye]->TextureCount;

			eyeFrameBuffer[eye]->SetDepthAttachement(eyeRenderTextureDepth[eye]);
			eyeFrameBuffer[eye]->SetColorAttachement(0, eyeRenderTextureList[eye][eyeTextureSet[eye]->CurrentIndex]);

			Render(m_renderer, eyeFrameBuffer[eye].get(), matProj, matMV);
			//glBindFramebuffer(GL_FRAMEBUFFER, 0);

			eyeFrameBuffer[eye]->SetColorAttachement(0, nullptr);
			eyeFrameBuffer[eye]->SetDepthAttachement(nullptr);

		}

		// Set up positional data.
		ovrViewScaleDesc viewScaleDesc;
		viewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;
		viewScaleDesc.HmdToEyeViewOffset[0] = ViewOffset[0];
		viewScaleDesc.HmdToEyeViewOffset[1] = ViewOffset[1];

		ovrLayerEyeFov ld;
		ld.Header.Type = ovrLayerType_EyeFov;
		ld.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;   // Because OpenGL.

		for (int eye = 0; eye < 2; ++eye)
		{
			ld.ColorTexture[eye] = eyeTextureSet[eye];
			ld.Viewport[eye] = OVR::Recti(eyeTextureSet[eye]->Textures[eyeTextureSet[eye]->CurrentIndex].Header.TextureSize);
			ld.Fov[eye] = hmdDesc.DefaultEyeFov[eye];
			ld.RenderPose[eye] = EyeRenderPose[eye];
		}

		ovrLayerHeader* layers = &ld.Header;
		ovrResult result = ovr_SubmitFrame(HMD, 0, &viewScaleDesc, &layers, 1);

		Render(m_renderer, NullFBO.get(), matProj, matMV);
#endif

		m_renderer->FinishFrame();
	}

	void OnKeyPress(int key)
	{
		if (key == GLFW_KEY_W)
			position += direction;
		else if (key == GLFW_KEY_S)
			position -= direction;
		else if (key == GLFW_KEY_A)
			position += right;
		else if (key == GLFW_KEY_D)
			position -= right;
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
				m_renderer->GetResourceFactory().ReloadResources(AT2::ReloadableGroup::Shaders);

			OnKeyPress(key);
		};

		m_window->KeyRepeatCallback = [&](int key)
		{
			OnKeyPress(key);
		};

		m_window->ResizeCallback = [&](const glm::ivec2& newSize)
		{
			m_framebufferPhysicalSize = newSize;
		};

		m_window->MouseUpCallback = [](int key)
		{
			std::cout << "Mouse " << key << std::endl;
		};

		m_window->MouseMoveCallback = [&](const MousePos& pos)
		{
			heading += pos.getDeltaPos().x * 0.01f;
			pitch += pos.getDeltaPos().y * 0.01f;
			pitch = glm::clamp(pitch, -glm::pi<float>() / 2, glm::pi<float>() / 2);

			direction = glm::normalize(glm::vec3(cos(pitch) * sin(heading), sin(pitch), cos(pitch) * cos(heading)));
		};

		m_window->InitializeCallback = [&]()
		{
			m_window->setVSyncInterval(1);
		};

		m_window->ClosingCallback = [&]()
		{
			m_renderer->Shutdown();
		};

		m_window->RenderCallback = std::bind(&App::OnRender, this, std::placeholders::_1);
		m_window->InitializeCallback = std::bind(&App::OnInitialize, this);
	}

private:
	std::shared_ptr<GlfwWindow> m_window;
	std::shared_ptr<AT2::GlRenderer> m_renderer;

	glm::ivec2 m_framebufferPhysicalSize = glm::ivec2(1024, 1024);

	
	//TODO: remake
	glm::mat4 matProj, matMV;
	float heading = 0.0f, pitch = 0.0f;
	glm::vec3 position = glm::vec3(0.0, 0.0, 0.0), direction;
	glm::vec3 right, up;
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
	
#ifdef AT2_USE_OCULUS_RIFT
	ovr_Destroy(HMD);
	OVR::System::Destroy();
#endif 

	return 0;
}
