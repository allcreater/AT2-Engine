#ifndef AT2_DRAWABLE_HEADER
#define AT2_DRAWABLE_HEADER

#include "AT2.h"

namespace AT2
{
	class [[deprecated]] IDrawable
	{
	public:
		virtual void Draw(const IRenderer& renderer) = 0;
		virtual ~IDrawable() {}
	};

}
#endif