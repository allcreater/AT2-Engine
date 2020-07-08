#include "TextureLoader.h"

#include "AT2.h"

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

//from OpenGL-friendly to platform-independ and back again at resources factory :)
static ExternalTextureFormat GetExternalFormat(ILuint externalFormat, ILuint externalType)
{
	auto convertType = [](ILenum type)
	{
		switch (type)
		{
		case IL_BYTE:			return BufferDataType::Byte;
		case IL_UNSIGNED_BYTE:	return BufferDataType::UByte;
		case IL_SHORT:			return BufferDataType::Short;
		case IL_UNSIGNED_SHORT:	return BufferDataType::UShort;
		case IL_INT:			return BufferDataType::Int;
		case IL_UNSIGNED_INT:	return BufferDataType::UInt;
		case IL_FLOAT:			return BufferDataType::Float;
		case IL_DOUBLE:			return BufferDataType::Double;
		case IL_HALF:			return BufferDataType::HalfFloat;
		default:
			assert(false);
			return BufferDataType::Byte;
		}
	};

	auto convertFormat = [](ILenum format)
	{
		switch (format)
		{
		case IL_ALPHA:
		case IL_LUMINANCE:
			Log::Info() << "Texture loader will treat alpha and luminance textures as Red" << std::endl;
			return TextureLayout::Red;
		case IL_RGB:
			return TextureLayout::RGB;
		case IL_RGBA:
			return TextureLayout::RGBA;
		case IL_BGR:
			return TextureLayout::BGR;
		case IL_BGRA:
			return TextureLayout::BGRA;
		case IL_LUMINANCE_ALPHA:
			Log::Info() << "Texture loader will convert alpha and luminance textures to RG" << std::endl;
			return TextureLayout::RG;
		default:
			assert(false);
			return TextureLayout::Red;
		}
	};

	return { convertFormat(externalFormat), convertType(externalType) };
}


static std::shared_ptr<ITexture> Load(const std::shared_ptr<IRenderer>& renderer, const std::function<bool()>& imageLoader)
{
	const bool enableAutomipmaps = true;

	const auto image = DevIL_Holder::Get().GenImage();

	ilBindImage(image.getId());

	if (!imageLoader())
		throw AT2Exception(AT2Exception::ErrorCase::File, "GlResourceFactory: could not load image");


	ILinfo imageInfo;
	iluGetImageInfo(&imageInfo);

    const auto size = glm::uvec3(imageInfo.Width, imageInfo.Height, imageInfo.Depth);

    const ILuint mipmapLevels = std::max(imageInfo.NumMips, 1u);
    const ILuint storageLevels = (imageInfo.NumMips || !enableAutomipmaps) ? imageInfo.NumMips : static_cast<ILuint>(log(std::max({ size.x, size.y, size.z })) / log(2));

	if (const auto flags = ilGetInteger(IL_IMAGE_CUBEFLAGS))//TODO: and how to live with it? :)
	{
		for (ILuint i = 0; i <= 5; ++i)
		{
			ilBindImage(image.getId());
			ilActiveImage(i);

			//...
		}

		throw AT2Exception(AT2Exception::ErrorCase::NotImplemented, "Cube maps still unsupported :(");
	}


	if (imageInfo.Depth > 1)
	{
		auto texture = renderer->GetResourceFactory().CreateTexture(Texture3D{ size, storageLevels }, GetExternalFormat(imageInfo.Format, imageInfo.Type));

		for (unsigned int level = 0; level < mipmapLevels; ++level)
		{
			ilActiveMipmap(level);

			ILinfo mipmapImageInfo;
			iluGetImageInfo(&mipmapImageInfo);

			texture->SubImage3D({ 0, 0, 0 }, { mipmapImageInfo.Width, mipmapImageInfo.Height, mipmapImageInfo.Depth }, level, GetExternalFormat(mipmapImageInfo.Format, mipmapImageInfo.Type), mipmapImageInfo.Data);
		}

		if (storageLevels != imageInfo.NumMips)
			texture->BuildMipmaps();

		return texture;
	}

	//Texture 2d
	auto texture = renderer->GetResourceFactory().CreateTexture(Texture2D{ glm::xy(size), storageLevels }, GetExternalFormat(imageInfo.Format, imageInfo.Type));

	for (unsigned int level = 0; level < mipmapLevels; ++level)
	{
		ilActiveMipmap(level);

		ILinfo mipmapImageInfo;
		iluGetImageInfo(&mipmapImageInfo);

		texture->SubImage2D({ 0, 0 }, { mipmapImageInfo.Width, mipmapImageInfo.Height }, level, GetExternalFormat(mipmapImageInfo.Format, mipmapImageInfo.Type), mipmapImageInfo.Data);
	}

	if (storageLevels != imageInfo.NumMips)
		texture->BuildMipmaps();

	return texture;
}

TextureRef TextureLoader::LoadTexture(const std::shared_ptr<IRenderer>& renderer, const str& filename)
{
	return Load(renderer, [filename]
	{
	    return ilLoadImage(filename.c_str()) == IL_TRUE;
	});
}

TextureRef TextureLoader::LoadTexture(const std::shared_ptr<IRenderer>& renderer, void* data, size_t size)
{
	if (size > std::numeric_limits<ILuint>::max())
		throw AT2Exception(AT2Exception::ErrorCase::Texture, "DevIL does not support images more than 4GB");

    const ILenum type = ilDetermineTypeL(data, static_cast<ILuint>(size));
	if (type == IL_TYPE_UNKNOWN)
		throw AT2Exception(AT2Exception::ErrorCase::Texture, "Couldn't determine texture format while reading from memory");

	return Load(renderer, [=]
	{
	    return ilLoadL(type, data, static_cast<ILuint>(size)) == IL_TRUE;
	});
}
