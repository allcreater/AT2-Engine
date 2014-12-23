#include "GlRenderer.h"
#include "GlTexture.h"

#include <gli\gli.hpp>

using namespace AT2;


std::shared_ptr<ITexture> GlResourceFactory::LoadTexture(const str& _filename) const
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

std::shared_ptr<ITexture> GlResourceFactory::CreateTexture() const
{
	throw AT2::AT2Exception("GlResourceFactory: not implemented"); //TODO!
}