#ifndef AT2_DRAWABLE_HEADER
#define AT2_DRAWABLE_HEADER

#include "AT2.h"

namespace AT2
{
	class [[deprecated]] IDrawable
	{
	public:
		virtual void Draw(const std::shared_ptr<IRenderer>& renderer) = 0;
		virtual ~IDrawable() {}

	private:

	};

}
#endif