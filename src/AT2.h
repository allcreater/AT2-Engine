#ifndef AT2_MAIN_HEADER_H
#define AT2_MAIN_HEADER_H

#include <exception>
#include <string>
#include <vector>
#include <set>
#include <memory>

#include "log.h"
#include "utils.hpp"

namespace AT2
{

typedef std::string str;

template <typename T>
class IBuffer
{
public:
	virtual void SetData(unsigned int length, const T* data) = 0;
	virtual T* Lock() = 0;
	virtual void Unlock() = 0;
};

class IFrameBuffer
{
public:
	virtual void Bind() = 0;
	virtual unsigned int GetId() const = 0;
	virtual ~IFrameBuffer() {};
};

class IVertexBuffer
{
public:
	virtual void Bind() = 0;
	virtual unsigned int GetId() const = 0;
	virtual unsigned int GetLength() = 0;
	virtual ~IVertexBuffer() {};
};

class IVertexArray
{
public:
	virtual void Bind() = 0;
	virtual unsigned int GetId() const = 0;
	virtual ~IVertexArray() {};
};

class ITexture
{
public:
	virtual void Bind(unsigned int module) = 0;
	virtual void Unbind() = 0;
	virtual void BuildMipmaps() = 0;

	virtual int GetCurrentModule() const  = 0;
	virtual unsigned int GetId() const = 0;
	
	virtual ~ITexture(){};

	struct BufferData
	{
		unsigned int Height, Width, Depth;
		void* Data;
	};
};

class IShaderProgram
{
public:
	virtual void Bind() = 0;
	virtual unsigned int GetId() const = 0;
	virtual ~IShaderProgram() {};
};

class IDrawPrimitive
{
public:
	virtual void Draw() const = 0;
	virtual ~IDrawPrimitive() {};
};

class AT2Exception : public std::exception
{
public:
	enum struct ErrorCase
	{
		Unknown = -1,
		Renderer,
		Shader,
		Buffer
	} Case;


	AT2Exception(const std::string& _message) : std::exception(_message.c_str()), Case(ErrorCase::Unknown) {};
	AT2Exception(ErrorCase _case, const std::string& _message) : std::exception(_message.c_str()), Case(_case) {};
};

typedef std::set<std::shared_ptr<ITexture>> TextureSet;
typedef std::vector<std::shared_ptr<IDrawPrimitive>> PrimitiveList;

class IStateManager
{
public:
	virtual void BindTextures(const TextureSet& textures) = 0;
	virtual void BindFramebuffer(const std::shared_ptr<IFrameBuffer>& framebuffer) = 0;
	virtual void BindShader(const std::shared_ptr<IShaderProgram>& shader) = 0;
	virtual void BindVertexArray(const std::shared_ptr<IVertexArray>& vertexArray) = 0;
};

class IRenderer
{
public:
	virtual ITexture* CreateTexture(unsigned int width, unsigned int height, unsigned int depth, void* data) = 0;

	virtual IStateManager* GetStateManager() = 0;

	virtual void Shutdown() = 0;
};

//Interface for all resources which can be dynamically reloaded from file
class IReloadable
{
public:
	virtual void Reload() = 0;
};

}

#endif