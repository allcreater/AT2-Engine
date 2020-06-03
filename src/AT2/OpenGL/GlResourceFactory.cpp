#include "GlRenderer.h"
#include "GlTexture.h"
#include "GlVertexArray.h"
#include "GlShaderProgram.h"

#include <fstream>



using namespace AT2;

/*
std::shared_ptr<ITexture> GlResourceFactory::LoadTexture_GLI(const str& _filename) const
{
#ifdef USE_GLI
	gli::storage Storage(gli::load_dds(_filename.c_str()));
	assert(!Storage.empty());

	bool enableAutomipmaps = true;

	if (Storage.layers() > 1)
	{
		gli::texture2DArray tex2d_array(Storage);

		unsigned int storageLevels = (tex2d_array.levels() > 1 || !enableAutomipmaps) ? tex2d_array.levels() : static_cast<unsigned int>(log(std::max(tex2d_array.dimensions().x, tex2d_array.dimensions().y)) / log(2));
		auto glTexture = std::make_shared<AT2::GlTexture2DArray>(
			gli::internal_format(tex2d_array.format()),
			glm::uvec3(tex2d_array.dimensions(), tex2d_array.layers()),
			storageLevels);

		AT2::GlTexture::BufferData bd;
		bd.ExternalFormat = gli::external_format(tex2d_array.format());
		bd.DataType = gli::type_format(tex2d_array.format());

		if (gli::is_compressed(tex2d_array.format()))
		{
			throw AT2::AT2Exception("GlResourceFactory: compressed textures is not supported yet"); //TODO!
		}
		else
		{
			for (unsigned int layer = 0; layer < tex2d_array.layers(); ++layer)
				for (unsigned int level = 0; level < tex2d_array.levels(); ++level)
				{
					bd.Height = tex2d_array[layer][level].dimensions().x;
					bd.Width = tex2d_array[layer][level].dimensions().y;
					bd.Depth = 1;
					bd.Data = tex2d_array[layer][level].data();

					glTexture->SetLayer(level, layer, bd);
				}
		}

		if (storageLevels != tex2d_array.levels())
			glTexture->BuildMipmaps();

		return glTexture;
	}
	else
	{
		gli::texture2D tex2d(Storage);

		unsigned int storageLevels = (tex2d.levels() > 1 || !enableAutomipmaps) ? tex2d.levels() : static_cast<unsigned int>(log(std::max(tex2d.dimensions().x, tex2d.dimensions().y)) / log(2));
		auto glTexture = std::make_shared<AT2::GlTexture2D>(gli::internal_format(tex2d.format()), tex2d.dimensions(), storageLevels);

		AT2::GlTexture::BufferData bd;
		bd.ExternalFormat = gli::external_format(tex2d.format());
		bd.DataType = gli::type_format(tex2d.format());

		if (gli::is_compressed(tex2d.format()))
		{
			throw AT2::AT2Exception("GlResourceFactory: compressed textures is not supported yet"); //TODO!
		}
		else
		{
			for (unsigned int level = 0; level < tex2d.levels(); ++level)
			{
				bd.Height = tex2d[level].dimensions().x;
				bd.Width = tex2d[level].dimensions().y;
				bd.Depth = 1;
				bd.Data = tex2d[level].data();

				glTexture->SetData(level, bd);
			}

			if (storageLevels != tex2d.levels())
				glTexture->BuildMipmaps();
		}
		return glTexture;
	}
#else
	return nullptr;
#endif
}
*/

GLint GlResourceFactory::GetInternalFormat(GLuint externalFormat, GLuint externalType)
{
	if (externalType == GL_UNSIGNED_BYTE)
	{
	  if (externalFormat == GL_RED)
			return GL_R8;
		else if (externalFormat == GL_RG)
			return GL_RG8;
		else if (externalFormat == GL_RGB)
			return GL_RGB8;
		else if (externalFormat == GL_RGBA)
			return GL_RGBA8;
		else
			throw AT2::AT2Exception("GlResourceFactory: not supported texture format?!");
	}
	else if (externalType == GL_FLOAT)
	{
		if (externalFormat == GL_RED)
			return GL_R32F;
		else if (externalFormat == GL_RG)
			return GL_RG32F;
		else if (externalFormat == GL_RGB)
			return GL_RGB32F;
		else if (externalFormat == GL_RGBA)
			return GL_RGBA32F;
		else
			throw AT2::AT2Exception("GlResourceFactory: not supported texture format?!");
	}
	else
		throw AT2::AT2Exception("GlResourceFactory: not supported texture data type");
}


GlResourceFactory::GlResourceFactory(GlRenderer* renderer) : m_renderer(renderer)
{
}

GlResourceFactory::~GlResourceFactory()
{
}

std::shared_ptr<ITexture> GlResourceFactory::CreateTextureFromFramebuffer(const glm::ivec2& pos, const glm::uvec2& size) const
{
	auto texture = std::make_shared<GlTexture2D>(GL_RGBA8, size);
	texture->CopyFromFramebuffer(0, pos, size);
	return texture;
}

std::shared_ptr<IVertexArray> GlResourceFactory::CreateVertexArray() const
{
	return std::make_shared<GlVertexArray>(m_renderer->GetRendererCapabilities());
}

std::shared_ptr<IVertexBuffer> GlResourceFactory::CreateVertexBuffer(VertexBufferType type, const BufferTypeInfo& dataType) const
{
	auto buffer = std::make_shared<GlVertexBuffer>(type);
	buffer->SetDataType(dataType);
	return buffer;
}

std::shared_ptr<IVertexBuffer> GlResourceFactory::CreateVertexBuffer(VertexBufferType type, const BufferTypeInfo& dataType, size_t dataLength, const void* data) const
{
	auto buffer = CreateVertexBuffer(type, dataType);
	buffer->SetData(dataLength, data);
	return buffer;
}

//TODO: detach file as a shader source from specific implementation, remove inheritance
std::shared_ptr<IShaderProgram> GlResourceFactory::CreateShaderProgramFromFiles(std::initializer_list<str> files) const
{

	class GlShaderProgramFromFileImpl : public GlShaderProgram, public virtual IReloadable
	{
	public:
		GlShaderProgramFromFileImpl(std::initializer_list<str> _shaders) : GlShaderProgram()
		{
			for (const auto& filename : _shaders)
			{
				if (GetName().empty())
					SetName(filename);

				if (filename.substr(filename.length() - 8) == ".vs.glsl")
					m_filenames.push_back(std::make_pair(filename, AT2::GlShaderType::Vertex));
				else if (filename.substr(filename.length() - 9) == ".tcs.glsl")
					m_filenames.push_back(std::make_pair(filename, AT2::GlShaderType::TesselationControl));
				else if (filename.substr(filename.length() - 9) == ".tes.glsl")
					m_filenames.push_back(std::make_pair(filename, AT2::GlShaderType::TesselationEvaluation));
				else if (filename.substr(filename.length() - 8) == ".gs.glsl")
					m_filenames.push_back(std::make_pair(filename, AT2::GlShaderType::Geometry));
				else if (filename.substr(filename.length() - 8) == ".fs.glsl")
					m_filenames.push_back(std::make_pair(filename, AT2::GlShaderType::Fragment));
				else
					throw AT2Exception(AT2Exception::ErrorCase::Shader, "unrecognized shader type"s);
			}

			Reload();
		}

		void Reload() override
		{
			GlShaderProgram::CleanUp();

			for (auto shader : m_filenames)
			{
				GlShaderProgram::AttachShader(LoadShader(shader.first), shader.second);
			}

			GlShaderProgram::Compile();
		}

		ReloadableGroup getReloadableClass() const override
		{
			return ReloadableGroup::Shaders;
		}

	private:
		std::string LoadShader(const str& _filename)
		{
			if (_filename.empty())
				return "";

			std::ifstream t(_filename);
			if (!t.is_open())
				throw AT2Exception(AT2Exception::ErrorCase::File, "file '"s + _filename + "' not found.");


			return std::string((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
		}

	private:
		std::vector<std::pair<str, AT2::GlShaderType>> m_filenames;

	};

	auto resource = std::make_shared<GlShaderProgramFromFileImpl>(files);
	m_reloadableResourcesList.push_back(std::weak_ptr<IReloadable>(resource));
	return resource;
}

void GlResourceFactory::ReloadResources(ReloadableGroup group)
{
	for (const auto& resource : m_reloadableResourcesList)
	{
		if (auto reloadable = resource.lock())
			if (reloadable->getReloadableClass() == group)
				reloadable->Reload();
	}
}
