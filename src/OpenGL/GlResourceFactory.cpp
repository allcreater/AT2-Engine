#include "GlRenderer.h"
#include "GlTexture.h"

#include <gli\gli.hpp>

using namespace AT2;


std::shared_ptr<ITexture> GlResourceFactory::LoadTexture(const str& _filename) const
{
	gli::storage Storage(gli::load_dds(_filename.c_str()));
	assert(!Storage.empty());

	if (Storage.layers() > 1)
	{
		gli::texture2DArray tex2d_array(Storage);

		if (gli::is_compressed(tex2d_array.format()))
		{
			throw AT2::AT2Exception("GlResourceFactory: compressed textures is not supported yet"); //TODO!
		}
		else
		{
			throw AT2::AT2Exception("GlResourceFactory: texture arrays is not supported yet"); //TODO!
		}
	}
	else
	{
		gli::texture2D tex2d(Storage);

		auto glTexture = std::make_shared<AT2::GlTexture2D>(gli::internal_format(tex2d.format()), gli::external_format(tex2d.format()), gli::type_format(tex2d.format()));

		if (gli::is_compressed(tex2d.format()))
		{
			throw AT2::AT2Exception("GlResourceFactory: compressed textures is not supported yet"); //TODO!
		}
		else
		{
			for (auto level = 0; level < tex2d.levels(); ++level)
			{
				AT2::ITexture::BufferData bd;
				bd.Height = tex2d[level].dimensions().x;
				bd.Width = tex2d[level].dimensions().y;
				bd.Depth = 1;
				bd.Data = tex2d[level].data();

				glTexture->UpdateData(GL_TEXTURE_2D, bd, level);
			}
		}
		return glTexture;
	}
}

std::shared_ptr<ITexture> GlResourceFactory::CreateTexture() const
{
	throw AT2::AT2Exception("GlResourceFactory: not implemented"); //TODO!
}