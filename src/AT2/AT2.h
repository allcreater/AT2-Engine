#ifndef AT2_MAIN_HEADER_H
#define AT2_MAIN_HEADER_H

//#define GLM_FORCE_SWIZZLE
#include <glm/gtx/vec_swizzle.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <string>
#include <string_view>
#include <vector>
#include <set>
#include <memory>
#include <variant>

#include "log.h"
#include "utils.hpp"


const double pi = std::acos(-1);

#include "AT2_types.hpp"
#include "AT2_textures.hpp"

using namespace std::literals;

namespace AT2
{

typedef std::string str;

enum class ReloadableGroup
{
	Shaders = 1,
	Textures = 2
};

//Interface for all resources which can be dynamically reloaded from file
class IReloadable
{
public:
	//because we want to reload resources by their types, not all together
	virtual ReloadableGroup getReloadableClass() const = 0;

	virtual ~IReloadable() {}

public:
	virtual void Reload() = 0;
};

//TODO: make it usable for vertex buffers, textures etc
class IBuffer
{
public:
	virtual ~IBuffer() = default;

public:
	virtual void SetData(unsigned int length, const void* data) = 0;
	//virtual std::unique_ptr<void*> Lock() = 0; //TODO: think about more useful wrapper
	//virtual void Unlock() = 0;
};

class ITexture;

class IFrameBuffer
{
public:
	IFrameBuffer() = default;
	IFrameBuffer(const IFrameBuffer& other) = delete;
	IFrameBuffer& operator= (const IFrameBuffer& other) = delete;
	virtual ~IFrameBuffer() = default;

public:
	virtual void Bind() = 0;
	virtual unsigned int GetId() const = 0;

	virtual void SetColorAttachement(unsigned int attachementNumber, std::shared_ptr<ITexture> texture) = 0;
	virtual std::shared_ptr<ITexture> GetColorAttachement(unsigned int attachementNumber) const = 0;
	virtual void SetDepthAttachement(const std::shared_ptr<ITexture> texture) = 0;
	virtual std::shared_ptr<ITexture> GetDepthAttachement() const = 0;

	virtual glm::ivec2 GetActualSize() const = 0;

};

class IVertexBuffer// : public IBuffer
{
public:
	IVertexBuffer() = default;
	IVertexBuffer(const IVertexBuffer& other) = delete;
	IVertexBuffer& operator= (const IVertexBuffer& other) = delete;
	virtual ~IVertexBuffer() = default;

public:
	virtual void Bind() = 0;

	virtual unsigned int GetId() const = 0;
	virtual size_t GetLength() const = 0;
	virtual VertexBufferType GetType() const = 0;

	virtual const BufferTypeInfo& GetDataType() const = 0;
	virtual void SetDataType(const BufferTypeInfo& typeInfo) = 0;

	virtual void SetData(size_t length, const void* data) = 0;
};

class IVertexArray
{
public:
	IVertexArray() = default;
	IVertexArray(const IVertexArray& other) = delete;
	IVertexArray& operator= (const IVertexArray& other) = delete;
	virtual ~IVertexArray() = default;

public:
	virtual void Bind() = 0;
	virtual unsigned int GetId() const = 0;

	virtual void SetIndexBuffer(const std::shared_ptr<IVertexBuffer>& buffer) = 0;
	virtual std::shared_ptr<IVertexBuffer> GetIndexBuffer() const = 0;
	//virtual std::shared_ptr<IVertexBuffer> GetOrSetIndexBuffer() const;
	virtual void SetVertexBuffer(unsigned int index, const std::shared_ptr<IVertexBuffer>& buffer) = 0;
	virtual void SetVertexBufferDivisor(unsigned int index, unsigned int divisor = 0) = 0;
	virtual std::shared_ptr<IVertexBuffer> GetVertexBuffer(unsigned int index) const = 0;
};

class ITexture
{
public:
	ITexture() = default;
	ITexture(const ITexture& other) = delete; //maybe temporary
	ITexture& operator= (const ITexture& other) = delete; //maybe temporary
	virtual ~ITexture() = default;

public:
	virtual void Bind(unsigned int module) = 0;
	virtual void Unbind() = 0;
	virtual void BuildMipmaps() = 0;

	virtual int GetCurrentModule() const  = 0;
	virtual unsigned int GetId() const = 0;
	virtual glm::uvec3 GetSize() const = 0;

	virtual const Texture& GetType() const = 0;
	virtual void SetWrapMode(TextureWrapMode wrapMode) = 0;
	virtual const TextureWrapMode& GetWrapMode() const = 0;

	//TODO: think how to make better
	virtual void SubImage1D(glm::u32 offset, glm::u32 size, glm::u32 level, ExternalTextureFormat dataFormat, void* data) = 0;
	virtual void SubImage2D(glm::uvec2 offset, glm::uvec2 size, glm::u32 level, ExternalTextureFormat dataFormat, void* data, int cubeMapFace = 0) = 0;
	virtual void SubImage3D(glm::uvec3 offset, glm::uvec3 size, glm::u32 level, ExternalTextureFormat dataFormat, void* data) = 0;
};

class IUniformContainer;

class IShaderProgram
{
public:
	IShaderProgram() = default;
	IShaderProgram(const IShaderProgram& other) = delete;
	IShaderProgram& operator= (const IShaderProgram& other) = delete;
	virtual ~IShaderProgram() = default;

public:
	virtual void Bind() = 0;
	virtual unsigned int GetId() const = 0;
	virtual bool IsActive() const = 0;
	virtual bool Compile() = 0;
	virtual std::shared_ptr<IUniformContainer> CreateAssociatedUniformStorage() = 0;
};

class IDrawPrimitive
{
public:
	virtual ~IDrawPrimitive() = default;
public:
	virtual void Draw() const = 0;

};

class IUniformContainer
{
public:
	virtual ~IUniformContainer() {}

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

	//unsigned integers
	virtual void SetUniform(const str& name, const glm::uint& value) = 0;
	virtual void SetUniform(const str& name, const glm::uvec2& value) = 0;
	virtual void SetUniform(const str& name, const glm::uvec3& value) = 0;
	virtual void SetUniform(const str& name, const glm::uvec4& value) = 0;

	//texture
	virtual void SetUniform(const str& name, std::weak_ptr<const ITexture> value) = 0;

	virtual void Bind() = 0;
};

class AT2Exception : public std::runtime_error
{
public:
	enum struct ErrorCase
	{
		Unknown = -1,
		NotImplemented,
		Renderer,
		Shader,
		Buffer,
		Texture,
		UI,
		File
	} Case;


	AT2Exception(const std::string& _message) : std::runtime_error(_message.c_str()), Case(ErrorCase::Unknown) {};
	AT2Exception(ErrorCase _case, const std::string& _message) : std::runtime_error(_message.c_str()), Case(_case) {};
};

class IRendererCapabilities
{
public:
	IRendererCapabilities() = default;
	IRendererCapabilities(const IRendererCapabilities& other) = delete;
	IRendererCapabilities& operator= (const IRendererCapabilities& other) = delete;
	virtual ~IRendererCapabilities() = default;

public:
	[[nodiscard]] virtual unsigned int GetMaxNumberOfTextureUnits() const = 0;
	[[nodiscard]] virtual unsigned int GetMaxTextureSize() const = 0;
	[[nodiscard]] virtual unsigned int GetMaxNumberOfVertexAttributes() const = 0;
	[[nodiscard]] virtual unsigned int GetMaxNumberOfColorAttachements() const = 0;
};

typedef std::set<std::shared_ptr<ITexture>> TextureSet;
typedef std::vector<IDrawPrimitive*> PrimitiveList;

class IStateManager
{
public:
	IStateManager() = default;
	IStateManager(const IStateManager& other) = delete;
	IStateManager& operator= (const IStateManager& other) = delete;
	virtual ~IStateManager() {}

public:
	virtual void BindTextures(const TextureSet& textures) = 0;
	virtual void BindFramebuffer(const std::shared_ptr<IFrameBuffer>& framebuffer) = 0;
	virtual void BindShader(const std::shared_ptr<IShaderProgram>& shader) = 0;
	virtual void BindVertexArray(const std::shared_ptr<IVertexArray>& vertexArray) = 0;

	//virtual TextureSet& GetActiveTextures() const = 0;
	virtual std::weak_ptr<IFrameBuffer> GetActiveFrameBuffer() const = 0;
	virtual std::weak_ptr<IShaderProgram> GetActiveShader() const = 0;
	virtual std::weak_ptr<IVertexArray> GetActiveVertexArray() const = 0;
};

class IResourceFactory
{
public:
	IResourceFactory() = default;
	IResourceFactory(const IResourceFactory& other) = delete;
	IResourceFactory& operator= (const IResourceFactory& other) = delete;

	virtual ~IResourceFactory() = default;

public:
	virtual std::shared_ptr<ITexture> CreateTextureFromFramebuffer(const glm::ivec2& pos, const glm::uvec2& size) const = 0; 
	virtual std::shared_ptr<ITexture> CreateTexture(const Texture& declaration, ExternalTextureFormat desiredFormat) const = 0;
	virtual std::shared_ptr<IVertexArray> CreateVertexArray() const = 0;
	virtual std::shared_ptr<IVertexBuffer> CreateVertexBuffer(VertexBufferType type, const BufferTypeInfo& dataType) const = 0;
	virtual std::shared_ptr<IVertexBuffer> CreateVertexBuffer(VertexBufferType type, const BufferTypeInfo& dataType, size_t dataLength, const void* data) const = 0;
	virtual std::shared_ptr<IShaderProgram> CreateShaderProgramFromFiles(std::initializer_list<str> files) const = 0;

	virtual void ReloadResources(ReloadableGroup group) = 0;
};

class IRenderer
{
public:
	virtual ~IRenderer() {};

public:
	virtual IResourceFactory& GetResourceFactory() const = 0;
	virtual IStateManager& GetStateManager() const = 0;
	virtual IRendererCapabilities& GetRendererCapabilities() const = 0;

	virtual void Shutdown() = 0;
	
	virtual void SetViewport(const AABB2d& viewport) = 0;
	virtual void ClearBuffer(const glm::vec4& color) = 0;
	virtual void ClearDepth(float depth) = 0;
	virtual void FinishFrame() = 0;

	virtual IFrameBuffer& GetDefaultFramebuffer() const = 0;
};


using TextureRef = std::shared_ptr<ITexture>;

}

#endif
