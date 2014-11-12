#ifndef AT2_DRAWABLE_CLASS
#define AT2_DRAWABLE_CLASS

#include "AT2.h"
#include "OpenGl\GlVertexArray.h"

using namespace AT2;

class IDrawable
{
public:
	virtual void Draw () = 0;

private:

};

class GlDrawable : public IDrawable
{
public:
	void Draw();

private:

};

#endif