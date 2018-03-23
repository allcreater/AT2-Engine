#include "GlRenderer.h"
#include "GlTexture.h"

#ifdef LIBRARY_GLI_AVAILABLE
#include <gli/gli.hpp>
#endif

#include <IL/il.h>
#include <IL/ilu.h>


#include <filesystem>

using namespace AT2;

//TODO add caching
std::shared_ptr<ITexture> GlResourceFactory::LoadTexture(const str& _filename) const
{
	using namespace std::tr2::sys;

	auto ext = path(_filename).extension();
	/*if (ext == "dds")
		return LoadTexture_GLI(_filename);
	else*/
		return LoadTexture_DevIL(_filename);
}

#ifdef LIBRARY_GLI_AVAILABLE
std::shared_ptr<ITexture> GlResourceFactory::LoadTexture_GLI(const str& _filename) const
{
	gli::storage Storage(gli::load_dds(_filename.c_str()));
	assert(!Storage.empty());

	bool enableAutomipmaps = true;

	if (Storage.layers() > 1)
	{
		gli::texture2DArray tex2d_array(Storage);

		int storageLevels = (tex2d_array.levels() > 1 || !enableAutomipmaps) ? tex2d_array.levels() : (log(std::max(tex2d_array.dimensions().x, tex2d_array.dimensions().y)) / log(2));
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
			for (auto layer = 0; layer < tex2d_array.layers(); ++layer)
				for (auto level = 0; level < tex2d_array.levels(); ++level)
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

		int storageLevels = (tex2d.levels() > 1 || !enableAutomipmaps) ? tex2d.levels() : (log(std::max(tex2d.dimensions().x, tex2d.dimensions().y)) / log(2));
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
			for (auto level = 0; level < tex2d.levels(); ++level)
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
}
#endif

GLint GlResourceFactory::GetInternalFormat(GLuint externalFormat, GLuint externalType) const
{
	if (externalType != GL_UNSIGNED_BYTE)
		throw AT2::AT2Exception("GlResourceFactory: not supported type");

	if (externalFormat == GL_RED)
	{
		return GL_R8;
	}
	else if (externalFormat == GL_RG)
	{
		return GL_RG8;
	}
	else if (externalFormat == GL_RGB)
	{
		return GL_RGB8;
	}
	else if (externalFormat == GL_RGBA)
	{
		return GL_RGBA8;
	}
	else
		throw AT2::AT2Exception("GlResourceFactory: not supported format?!");
}

std::shared_ptr<ITexture> GlResourceFactory::LoadTexture_DevIL(const str& filename) const
{
	const bool enableAutomipmaps = true;

	ILuint imageID = ilGenImage();
	ilBindImage(imageID);

	if (ilLoadImage(filename.c_str()) == IL_TRUE)
	{
		ILinfo imageInfo;
		iluGetImageInfo(&imageInfo);

		AT2::GlTexture::BufferData bd;
		bd.ExternalFormat = imageInfo.Format;
		bd.DataType = imageInfo.Type;

		auto size = glm::uvec3(imageInfo.Width, imageInfo.Height, imageInfo.Depth);

		ILuint mipmapLevels = std::max(imageInfo.NumMips, 1u);
		ILuint storageLevels = (imageInfo.NumMips || !enableAutomipmaps) ? imageInfo.NumMips : (log(std::max({ size.x, size.y, size.z })) / log(2));


		if (imageInfo.Depth > 1)
		{
			auto glTexture = std::make_shared<AT2::GlTexture3D>(GetInternalFormat(imageInfo.Format, imageInfo.Type), size, storageLevels);
			
			for (auto level = 0; level < mipmapLevels; ++level)
			{
				ilActiveMipmap(level);
			
				ILinfo mipmapImageInfo;
				iluGetImageInfo(&mipmapImageInfo);

				bd.Height = mipmapImageInfo.Height;
				bd.Width = mipmapImageInfo.Width;
				bd.Depth = mipmapImageInfo.Depth;
				bd.Data = mipmapImageInfo.Data;
				bd.DataType = mipmapImageInfo.Type;

				glTexture->SetData(level, bd);
			}

			if (storageLevels != imageInfo.NumMips)
				glTexture->BuildMipmaps();

			return glTexture;
		}
		else
		{
			auto glTexture = std::make_shared<AT2::GlTexture2D>(GetInternalFormat(imageInfo.Format, imageInfo.Type), size.xy, storageLevels);

			for (auto level = 0; level < mipmapLevels; ++level)
			{
				ilActiveMipmap(level);

				ILinfo mipmapImageInfo;
				iluGetImageInfo(&mipmapImageInfo);

				bd.Height = mipmapImageInfo.Height;
				bd.Width = mipmapImageInfo.Width;
				bd.Depth = 1;
				bd.Data = mipmapImageInfo.Data;
				bd.DataType = mipmapImageInfo.Type;

				glTexture->SetData(level, bd);
			}

			if (storageLevels != imageInfo.NumMips)
				glTexture->BuildMipmaps();

			return glTexture;
		}
	}
	else
		throw AT2::AT2Exception("GlResourceFactory: could not load image");


	ilDeleteImage(imageID);
}

GlResourceFactory::GlResourceFactory()
{
	ilInit();
	iluInit();

	ilEnable(IL_ORIGIN_SET);
	ilSetInteger(IL_ORIGIN_MODE, IL_ORIGIN_UPPER_LEFT);
}

GlResourceFactory::~GlResourceFactory()
{
	ilShutDown();
}

std::shared_ptr<ITexture> GlResourceFactory::CreateTexture() const
{
	throw AT2::AT2Exception("GlResourceFactory: not implemented"); //TODO!
}