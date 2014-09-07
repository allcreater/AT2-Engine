#ifndef AT2_GL_FRAMEBUFFER_H
#define AT2_GL_FRAMEBUFFER_H

#include "AT2lowlevel.h"

namespace AT2
{

class GlFrameBuffer : public IFrameBuffer
{
public:
	virtual void Bind();
	virtual unsigned int GetId() const { return m_id; }

private:
	GLuint m_id;
};

}
#endif