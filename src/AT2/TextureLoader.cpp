#include "TextureLoader.h"

#include "OpenGL/GlTexture.h"

#ifdef USE_GLI
#include <gli/gli.hpp>
#endif

#include <algorithm>

#include <IL/il.h>
#include <IL/ilu.h>

class IlImage
{
public:

	IlImage() : m_id( ilGenImage() ) {}
	~IlImage() { ilDeleteImage(m_id); }

	[[nodiscard]] ILuint getId() const noexcept { return m_id; }

private:
	ILuint m_id;
};

class DevIL_Holder final
{
	DevIL_Holder()
	{
		ilInit();
		iluInit();

		ilEnable(IL_ORIGIN_SET);
		ilSetInteger(IL_ORIGIN_MODE, IL_ORIGIN_UPPER_LEFT);
	}

	~DevIL_Holder()
	{
		ilShutDown();
	}

public:
	IlImage GenImage()
	{
		return {};
	}

	static DevIL_Holder& Get()
	{
		static DevIL_Holder holder;
		return holder;
	}
};

using namespace AT2;


static GLint GetInternalFormat(GLuint externalFormat, GLuint externalType)
{
	if (externalType == GL_UNSIGNED_BYTE)
	{
		if (externalFormat == GL_RED)
			return GL_R8;
        if (externalFormat == GL_RG)
            return GL_RG8;
        if (externalFormat == GL_RGB)
            return GL_RGB8;
        if (externalFormat == GL_RGBA)
            return GL_RGBA8;
        
        throw AT2Exception("GlResourceFactory: not supported texture format?!");
    }

	if (externalType == GL_FLOAT)
	{
		if (externalFormat == GL_RED)
			return GL_R32F;
		if (externalFormat == GL_RG)
			return GL_RG32F;
		if (externalFormat == GL_RGB)
			return GL_RGB32F;
		if (externalFormat == GL_RGBA)
			return GL_RGBA32F;
		
		throw AT2Exception("GlResourceFactory: not supported texture format?!");
	}
	
	throw AT2Exception("GlResourceFactory: not supported texture data type");
}


static std::shared_ptr<ITexture> Load(std::shared_ptr<IRenderer> renderer, std::function<bool()> imageLoader)
{
	const bool enableAutomipmaps = true;

	const auto image = DevIL_Holder::Get().GenImage();

	ilBindImage(image.getId());

	if (!imageLoader())
		throw AT2Exception(AT2Exception::ErrorCase::File, "GlResourceFactory: could not load image");


	ILinfo imageInfo;
	iluGetImageInfo(&imageInfo);

	GlTexture::BufferData bd;
	bd.ExternalFormat = imageInfo.Format;

	auto size = glm::uvec3(imageInfo.Width, imageInfo.Height, imageInfo.Depth);

	ILuint mipmapLevels = std::max(imageInfo.NumMips, 1u);
	ILuint storageLevels = (imageInfo.NumMips || !enableAutomipmaps) ? imageInfo.NumMips : static_cast<ILuint>(log(std::max({ size.x, size.y, size.z })) / log(2));


	if (imageInfo.Depth > 1)
	{
		auto glTexture = std::make_shared<AT2::GlTexture3D>(GetInternalFormat(imageInfo.Format, imageInfo.Type), size, storageLevels);

		for (unsigned int level = 0; level < mipmapLevels; ++level)
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

	auto glTexture = std::make_shared<AT2::GlTexture2D>(GetInternalFormat(imageInfo.Format, imageInfo.Type), glm::xy(size), storageLevels);

	for (unsigned int level = 0; level < mipmapLevels; ++level)
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

//TODO: make API-independ
TextureRef TextureLoader::LoadTexture(std::shared_ptr<IRenderer> renderer, const str& filename)
{
	return Load(std::move(renderer), [filename]
	{
	    return ilLoadImage(filename.c_str()) == IL_TRUE;
	});
}

TextureRef TextureLoader::LoadTexture(std::shared_ptr<IRenderer> renderer, void* data, size_t size)
{
	ILenum type = ilDetermineTypeL(data, size);
	if (type == IL_TYPE_UNKNOWN)
		throw AT2Exception(AT2Exception::ErrorCase::Texture, "Couldn't determine texture format while reading from memory");

	return Load(std::move(renderer), [=]
	{
	    return ilLoadL(type, data, size) == IL_TRUE;
	});
}
