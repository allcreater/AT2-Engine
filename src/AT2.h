#ifndef AT2_MAIN_HEADER_H
#define AT2_MAIN_HEADER_H

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>


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

	virtual ~IBuffer() {};
};

class IFrameBuffer
{
public:
	IFrameBuffer() = default;
	IFrameBuffer(const IFrameBuffer& other) = delete;
	IFrameBuffer& operator= (const IFrameBuffer& other) = delete;

public:
	virtual void Bind() = 0;
	virtual unsigned int GetId() const = 0;
	virtual ~IFrameBuffer() {};
};

class IVertexBuffer
{
public:
	IVertexBuffer() = default;
	IVertexBuffer(const IVertexBuffer& other) = delete;
	IVertexBuffer& operator= (const IVertexBuffer& other) = delete;

public:
	virtual void Bind() = 0;
	virtual unsigned int GetId() const = 0;
	virtual unsigned int GetLength() = 0;
	virtual ~IVertexBuffer() {};
};

class IVertexArray
{
public:
	IVertexArray() = default;
	IVertexArray(const IVertexArray& other) = delete;
	IVertexArray& operator= (const IVertexArray& other) = delete;

public:
	virtual void Bind() = 0;
	virtual unsigned int GetId() const = 0;
	virtual ~IVertexArray() {};
};

class ITexture
{
public:
	ITexture() = default;
	ITexture(const ITexture& other) = delete;
	ITexture& operator= (const ITexture& other) = delete;

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
	IShaderProgram() = default;
	IShaderProgram(const IShaderProgram& other) = delete;
	IShaderProgram& operator= (const IShaderProgram& other) = delete;

public:
	virtual void Bind() = 0;
	virtual unsigned int GetId() const = 0;
	virtual bool IsActive() const = 0;
	virtual ~IShaderProgram() {};
};

class IDrawPrimitive
{
public:
	virtual void Draw() const = 0;
	virtual ~IDrawPrimitive() {};
};

class IUniformContainer
{
public:
	//doubles
	virtual void SetUniform(const str& name, const double& value) = 0;
	virtual void SetUniform(const str& name, const glm::dvec2& value) = 0;
	virtual void SetUniform(const str& name, const glm::dvec3& value) = 0;
	virtual void SetUniform(const str& name, const glm::dvec4& value) = 0;

	virtual void SetUniform(const str& name, const glm::dmat2& value) = 0;
	virtual void SetUniform(const str& name, const glm::dmat3& value) = 0;
	virtual void SetUniform(const str& name, const glm::dmat4& value) = 0;

	//floats
	virtual void SetUniform(const str& name, const float& value) = 0;
	virtual void SetUniform(const str& name, const glm::vec2& value) = 0;
	virtual void SetUniform(const str& name, const glm::vec3& value) = 0;
	virtual void SetUniform(const str& name, const glm::vec4& value) = 0;

	virtual void SetUniform(const str& name, const glm::mat2& value) = 0;
	virtual void SetUniform(const str& name, const glm::mat3& value) = 0;
	virtual void SetUniform(const str& name, const glm::mat4& value) = 0;

	//integers
	virtual void SetUniform(const str& name, const int& value) = 0;
	virtual void SetUniform(const str& name, const glm::ivec2& value) = 0;
	virtual void SetUniform(const str& name, const glm::ivec3& value) = 0;
	virtual void SetUniform(const str& name, const glm::ivec4& value) = 0;

	//texture
	virtual void SetUniform(const str& name, std::weak_ptr<const ITexture> value) = 0;

	virtual void Bind() = 0;
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

class IRendererCapabilities
{
public:
	virtual unsigned int GetMaxNumberOfTextureUnits() const = 0;
	virtual unsigned int GetMaxTextureSize() const = 0;
	virtual unsigned int GetMaxNumberOfVertexAttributes() const = 0;
	virtual unsigned int GetMaxNumberOfColorAttachements() const = 0;
};

typedef std::set<std::shared_ptr<ITexture>> TextureSet;
typedef std::vector<IDrawPrimitive*> PrimitiveList;

class IStateManager
{
public:
	virtual void BindTextures(const TextureSet& textures) = 0;
	virtual void BindFramebuffer(const std::shared_ptr<IFrameBuffer>& framebuffer) = 0;
	virtual void BindShader(const std::shared_ptr<IShaderProgram>& shader) = 0;
	virtual void BindVertexArray(const std::shared_ptr<IVertexArray>& vertexArray) = 0;
};

class IResourceFactory
{
public:
	virtual std::shared_ptr<ITexture> LoadTexture(const str& filename) const = 0; //TODO: maybe I need to detach load functionality 
	virtual std::shared_ptr<ITexture> CreateTexture() const = 0;
};

class IRenderer
{
public:
	virtual IResourceFactory* GetResourceFactory() const = 0;
	virtual IStateManager* GetStateManager() const = 0;
	virtual IRendererCapabilities* GetRendererCapabilities() const = 0;

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