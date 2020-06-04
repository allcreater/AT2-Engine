#include "MeshLoader.h"


#include <filesystem>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/pbrmaterial.h>

#include <GL/glew.h>

#include "../drawable.h"
#include <AT2/OpenGL/GlDrawPrimitive.h>
#include <AT2/TextureLoader.h>
using namespace AT2;


class GlMeshBuilder
{
public:
	GlMeshBuilder(const std::shared_ptr<AT2::IRenderer> _renderer, const aiScene* scene) :
        m_renderer(std::move(_renderer)),
        m_scene(scene)
	{
	}

public:
	std::shared_ptr <MeshNode> m_buildedMesh;

	std::shared_ptr <MeshNode> Build(std::shared_ptr<IShaderProgram> program)
	{
		m_buildedMesh = std::make_shared<MeshNode>();
		m_buildedMesh->SetName("Root");

		m_buildedMesh->Shader = program;
		m_buildedMesh->UniformBuffer = program->CreateAssociatedUniformStorage();

		BuildVAO();
		TraverseNode(m_scene->mRootNode, m_buildedMesh);

		return std::move(m_buildedMesh);
	}

protected:
	std::filesystem::path m_scenePath;
	Assimp::Importer m_importer;
	const aiScene* m_scene;
	std::shared_ptr<IRenderer> m_renderer;


	std::vector<glm::vec3> m_verticesVec;
	std::vector<glm::vec3> m_texCoordVec;
	std::vector<glm::vec3> m_normalsVec;
	std::vector<GLuint> m_indicesVec;

	std::vector<GLint> m_meshIndexOffsets;

protected:
	void AddMesh(const aiMesh* _mesh)
	{
		const auto vertexOffset = m_verticesVec.size();

		m_meshIndexOffsets.push_back(m_indicesVec.size());

		m_verticesVec.insert(m_verticesVec.end(), reinterpret_cast<glm::vec3*>(_mesh->mVertices), reinterpret_cast<glm::vec3*>(_mesh->mVertices) + _mesh->mNumVertices);
		m_texCoordVec.insert(m_texCoordVec.end(), reinterpret_cast<glm::vec3*>(_mesh->mTextureCoords[0]), reinterpret_cast<glm::vec3*>(_mesh->mTextureCoords[0]) + _mesh->mNumVertices);
		m_normalsVec.insert(m_normalsVec.end(), reinterpret_cast<glm::vec3*>(_mesh->mNormals), reinterpret_cast<glm::vec3*>(_mesh->mNormals) + _mesh->mNumVertices);

		for (int j = 0; j < _mesh->mNumFaces; ++j)
		{
			const aiFace& face = _mesh->mFaces[j];
			assert(face.mNumIndices == 3);

			m_indicesVec.push_back(face.mIndices[0] + vertexOffset);
			m_indicesVec.push_back(face.mIndices[2] + vertexOffset);
			m_indicesVec.push_back(face.mIndices[1] + vertexOffset);
		}
	}

	void BuildVAO()
	{
		for (unsigned i = 0; i < m_scene->mNumMeshes; ++i)
		{
			AddMesh(m_scene->mMeshes[i]);
		}

		auto& rf = m_renderer->GetResourceFactory();
		auto vao = rf.CreateVertexArray();
		vao->SetVertexBuffer(1, rf.CreateVertexBuffer(AT2vbt::ArrayBuffer, AT2::BufferDataTypes::Vec3, m_verticesVec.size() * sizeof(glm::vec3), m_verticesVec.data()));
		vao->SetVertexBuffer(2, rf.CreateVertexBuffer(AT2vbt::ArrayBuffer, AT2::BufferDataTypes::Vec3, m_texCoordVec.size() * sizeof(glm::vec3), m_texCoordVec.data()));
		vao->SetVertexBuffer(3, rf.CreateVertexBuffer(AT2vbt::ArrayBuffer, AT2::BufferDataTypes::Vec3, m_normalsVec.size() * sizeof(glm::vec3), m_normalsVec.data()));
		vao->SetIndexBuffer(rf.CreateVertexBuffer(AT2vbt::IndexBuffer, AT2::BufferDataTypes::UInt, m_indicesVec.size() * sizeof(GLuint), m_indicesVec.data()));

		m_buildedMesh->VertexArray = vao;
	}

	void TranslateMaterial(const aiMaterial* material, std::shared_ptr<DrawableNode> node)
	{

		const static std::tuple<aiTextureType, unsigned, std::string_view> knownTextureFlavours[] =
		{
			{aiTextureType_DIFFUSE, 0, "u_texAlbedo"sv},
			{AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_TEXTURE, "u_texAlbedo"sv},
			{aiTextureType_NORMALS, 0, "u_texNormalMap"sv},
			{aiTextureType_HEIGHT, 0, "u_texNormalMap"sv},
			{AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, "u_texAoRoughnessMetallic"sv}
		};

		auto loadTexture = [=](const char* path) -> TextureRef
		{
			if (const aiTexture* embeddedTexture = m_scene->GetEmbeddedTexture(path))
			{
				if (embeddedTexture->mHeight)
				{
					throw AT2Exception(AT2Exception::ErrorCase::Texture, "reading raw texture from memory not implemented yet");
				}
				else
				{
					return TextureLoader::LoadTexture(m_renderer, embeddedTexture->pcData, embeddedTexture->mWidth);
				}
			}

			//it's not embedded
			return TextureLoader::LoadTexture(m_renderer, path);
		};

		std::map<str, std::shared_ptr<ITexture>, std::less<>> textures;

		for (auto [type, index, name] : knownTextureFlavours)
		{
			if (textures.find(name) != textures.end())
				continue;

			if (aiString path; material->GetTexture(type, index, &path) == aiReturn_SUCCESS && path.length > 0)
			{
				if (auto texture = loadTexture(path.C_Str()))
				{
					textures.emplace(name, texture);
					node->UniformBuffer->SetUniform(str{name}, texture);
					node->Textures.emplace(std::move(texture));
				}
			}
		}
	}

	void TraverseNode(const aiNode* node, std::shared_ptr<Node> baseNode)
	{
		if (node->mNumMeshes)
		{
			for (unsigned i = 0; i < node->mNumMeshes; i++)
			{
				const int meshIndex = node->mMeshes[i];
				const aiMesh* mesh = m_scene->mMeshes[meshIndex];
				const aiMaterial* material = m_scene->mMaterials[mesh->mMaterialIndex];

				//TODO: don't multiply nodes when materials are the same
			    auto submesh = std::make_shared<DrawableNode>();
				submesh->SetName("Submesh "s + mesh->mName.C_Str());
				submesh->SetTransform(ConvertMatrix(node->mTransformation));
                submesh->UniformBuffer = m_buildedMesh->Shader->CreateAssociatedUniformStorage();

				TranslateMaterial(material, submesh);
				submesh->Primitives.push_back(std::make_unique<GlDrawElementsPrimitive>(
					GlDrawPrimitiveType::Triangles, 
					mesh->mNumFaces * 3, 
					GlDrawElementsPrimitive::IndicesType::UnsignedInt, 
					m_meshIndexOffsets[meshIndex] * sizeof(GLuint)
				));

				baseNode->AddChild(std::move(submesh));
			}
		}
		//else
		//{
		//	transform = ConvertMatrix(node->mTransformation) * transform;
		//}

		for (int i = 0; i < node->mNumChildren; i++)
		{
			auto* children = node->mChildren[i];

			auto subnode = std::make_shared<Node>();
			subnode->SetName(children->mName.C_Str());
			baseNode->AddChild(subnode);

			TraverseNode(children, std::move(subnode));
		}
	}

	static glm::mat4 ConvertMatrix(const aiMatrix4x4& _t)
	{
		return glm::transpose(glm::make_mat4(&_t.a1));
	}

};

constexpr uint32_t flags =  
    aiProcess_GenSmoothNormals | //can't be specified with aiProcess_GenNormals
	aiProcess_FindInstances |
	aiProcess_FindDegenerates |
	aiProcess_FindInvalidData |
	aiProcess_CalcTangentSpace |
	aiProcess_ValidateDataStructure |
	aiProcess_OptimizeMeshes |
	aiProcess_OptimizeGraph |
	aiProcess_JoinIdenticalVertices |
	aiProcess_ImproveCacheLocality |
	aiProcess_LimitBoneWeights |
	aiProcess_RemoveRedundantMaterials |
	aiProcess_SplitLargeMeshes |
	aiProcess_Triangulate |
	aiProcess_GenUVCoords |
	aiProcess_GenBoundingBoxes |
	aiProcess_SplitByBoneCount |
	aiProcess_SortByPType |
	aiProcess_FlipUVs;


NodeRef MeshLoader::LoadNode(std::shared_ptr<IRenderer> renderer, const str& filename, std::shared_ptr<IShaderProgram> program)
{
    Assimp::Importer importer;
    auto* scene = importer.ReadFile(filename, flags);

	GlMeshBuilder builder { renderer, scene };
	

    return builder.Build(std::move(program));
}
