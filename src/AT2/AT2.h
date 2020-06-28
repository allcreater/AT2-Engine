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

#define NON_COPYABLE_OR_MOVABLE(type) \
    type(const type&) = delete; \
    type(type&&) = delete; \
    type& operator=(const type&) = delete; \
    type& operator=(type&&) = delete; \

namespace AT2
{

typedef std::string str;

class IReloadable;
class IBuffer;
class IFrameBuffer;
class IVertexBuffer;
class IVertexArray;
class ITexture;
class IShaderProgram;
class IUniformContainer;
class IRendererCapabilities;
class IStateManager;
class IResourceFactory;
class IRenderer;

enum class ReloadableGroup
{
	Shaders = 1,
	Textures = 2
};

//TODO: think about it's fate :)
//Interface for all resources which can be dynamically reloaded from file
class IReloadable
{
public:
	//because we want to reload resources by their types, not all together
    [[nodiscard]] virtual ReloadableGroup getReloadableClass() const = 0;

	virtual ~IReloadable() = default;

public:
	virtual void Reload() = 0;
};

//TODO: make it usable for vertex buffers, textures etc
class IBuffer
{
public:
	virtual ~IBuffer() = default;

public:
    [[nodiscard]] virtual size_t GetLength() const = 0;
	virtual void SetData(size_t length, const void* data) = 0;
	//virtual std::unique_ptr<void*> Lock() = 0; //TODO: think about more useful wrapper
	//virtual void Unlock() = 0;
};

class IFrameBuffer
{
public:
	NON_COPYABLE_OR_MOVABLE(IFrameBuffer)

	IFrameBuffer() = default;
	virtual ~IFrameBuffer() = default;

public:
	virtual void Bind() = 0;
    [[nodiscard]] virtual unsigned int GetId() const = 0;

	virtual void SetColorAttachment(unsigned int attachmentNumber, const std::shared_ptr<ITexture>& texture) = 0;
    [[nodiscard]] virtual std::shared_ptr<ITexture> GetColorAttachment(unsigned int attachmentNumber) const = 0;
	virtual void SetDepthAttachment(const std::shared_ptr<ITexture>& texture) = 0;
    [[nodiscard]] virtual std::shared_ptr<ITexture> GetDepthAttachment() const = 0;

    [[nodiscard]] virtual glm::ivec2 GetActualSize() const = 0;

};

class IVertexBuffer : public IBuffer
{
public:
	NON_COPYABLE_OR_MOVABLE(IVertexBuffer)

	IVertexBuffer() = default;
	virtual ~IVertexBuffer() = default;

public:
	virtual void Bind() = 0;

    [[nodiscard]] virtual unsigned int GetId() const = 0;
    [[nodiscard]] virtual VertexBufferType GetType() const = 0;

    [[nodiscard]] virtual const BufferTypeInfo& GetDataType() const = 0;
	virtual void SetDataType(const BufferTypeInfo& typeInfo) = 0;
};

class IVertexArray
{
public:
	NON_COPYABLE_OR_MOVABLE(IVertexArray)

	IVertexArray() = default;
	virtual ~IVertexArray() = default;

public:
	virtual void Bind() = 0;
    [[nodiscard]] virtual unsigned int GetId() const = 0;

	virtual void SetIndexBuffer(std::shared_ptr<IVertexBuffer> buffer) = 0;
    [[nodiscard]] virtual std::shared_ptr<IVertexBuffer> GetIndexBuffer() const = 0;
	//virtual std::shared_ptr<IVertexBuffer> GetOrSetIndexBuffer() const;
	//TODO GetOrCreateVertexBuffer ?
	virtual void SetVertexBuffer(unsigned int index, std::shared_ptr<IVertexBuffer> buffer) = 0;
	virtual void SetVertexBufferDivisor(unsigned int index, unsigned int divisor = 0) = 0;
    [[nodiscard]] virtual std::shared_ptr<IVertexBuffer> GetVertexBuffer(unsigned int index) const = 0;
};

class ITexture
{
public:
	NON_COPYABLE_OR_MOVABLE(ITexture)

	ITexture() = default;
	virtual ~ITexture() = default;

public:
	virtual void Bind(unsigned int module) const = 0;
	//TODO: think about interface
	virtual void BindAsImage(unsigned int module, glm::u32 level, glm::u32 layer, bool isLayered, BufferUsage usage = BufferUsage::ReadWrite) const = 0;
	virtual void Unbind() const = 0;
	virtual void BuildMipmaps() = 0;

    [[nodiscard]] virtual int GetCurrentModule() const noexcept = 0;
    [[nodiscard]] virtual unsigned int GetId() const noexcept = 0;
    [[nodiscard]] virtual glm::uvec3 GetSize() const noexcept = 0;
    [[nodiscard]] virtual size_t GetDataLength() const noexcept = 0;

    [[nodiscard]] virtual const Texture& GetType() const noexcept = 0;
	virtual void SetWrapMode(TextureWrapMode wrapMode) = 0;
    [[nodiscard]] virtual const TextureWrapMode& GetWrapMode() const = 0;

	//TODO: think how to make better
	virtual void SubImage1D(glm::u32 offset, glm::u32 size, glm::u32 level, ExternalTextureFormat dataFormat, void* data) = 0;
	virtual void SubImage2D(glm::uvec2 offset, glm::uvec2 size, glm::u32 level, ExternalTextureFormat dataFormat, void* data, int cubeMapFace = 0) = 0;
	virtual void SubImage3D(glm::uvec3 offset, glm::uvec3 size, glm::u32 level, ExternalTextureFormat dataFormat, void* data) = 0;
};

class IShaderProgram
{
public:
	NON_COPYABLE_OR_MOVABLE(IShaderProgram)

	IShaderProgram() = default;
	virtual ~IShaderProgram() = default;

public:
	virtual void Bind() = 0;
    [[nodiscard]] virtual unsigned int GetId() const = 0;
    [[nodiscard]] virtual bool IsActive() const = 0;
	virtual bool Compile() = 0;
	virtual std::shared_ptr<IUniformContainer> CreateAssociatedUniformStorage() = 0;

	//Warning: Shader reloading/relinking will invalidate that state
	virtual void	SetUBO(const str& blockName, unsigned int index) = 0;
	virtual void	SetUniform(const str& name, Uniform value) = 0;
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
	NON_COPYABLE_OR_MOVABLE(IUniformContainer)

	IUniformContainer() = default;
	virtual ~IUniformContainer() = default;

	virtual void SetUniform(const str& name, const Uniform& value) = 0;

	//texture
	virtual void SetUniform(const str& name, const std::shared_ptr<ITexture> &value) = 0;

	virtual void Bind(IStateManager &stateManager) const = 0;
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
	NON_COPYABLE_OR_MOVABLE(IRendererCapabilities)
	IRendererCapabilities() = default;
	virtual ~IRendererCapabilities() = default;

public:
	[[nodiscard]] virtual unsigned int GetMaxNumberOfTextureUnits() const = 0;
	[[nodiscard]] virtual unsigned int GetMaxTextureSize() const = 0;
	[[nodiscard]] virtual unsigned int GetMaxNumberOfVertexAttributes() const = 0;
	[[nodiscard]] virtual unsigned int GetMaxNumberOfColorAttachments() const = 0;
};

typedef std::set<std::shared_ptr<const ITexture>> TextureSet;
typedef std::vector<IDrawPrimitive*> PrimitiveList;

class IStateManager
{
public:
	NON_COPYABLE_OR_MOVABLE(IStateManager)

	IStateManager() = default;
	virtual ~IStateManager() = default;

public:
	virtual void BindTextures(const TextureSet& textures) = 0;
	virtual void BindFramebuffer(const std::shared_ptr<IFrameBuffer>& framebuffer) = 0;
	virtual void BindShader(const std::shared_ptr<IShaderProgram>& shader) = 0;
	virtual void BindVertexArray(const std::shared_ptr<IVertexArray>& vertexArray) = 0;

	//virtual TextureSet& GetActiveTextures() const = 0;
    [[nodiscard]] virtual std::shared_ptr<IFrameBuffer> GetActiveFrameBuffer() const = 0;
    [[nodiscard]] virtual std::shared_ptr<IShaderProgram> GetActiveShader() const = 0;
    [[nodiscard]] virtual std::shared_ptr<IVertexArray> GetActiveVertexArray() const = 0;

    [[nodiscard]] virtual std::optional<BufferDataType> GetIndexDataType() const noexcept = 0;
};

class IResourceFactory
{
public:
	NON_COPYABLE_OR_MOVABLE(IResourceFactory)

	IResourceFactory() = default;
	virtual ~IResourceFactory() = default;

public:
    [[nodiscard]] virtual std::shared_ptr<ITexture> CreateTextureFromFramebuffer(const glm::ivec2& pos, const glm::uvec2& size) const = 0;
    [[nodiscard]] virtual std::shared_ptr<ITexture> CreateTexture(const Texture& declaration, ExternalTextureFormat desiredFormat) const = 0;
    [[nodiscard]] virtual std::shared_ptr<IVertexArray> CreateVertexArray() const = 0;
    [[nodiscard]] virtual std::shared_ptr<IVertexBuffer> CreateVertexBuffer(VertexBufferType type, const BufferTypeInfo& dataType) const = 0;
	virtual std::shared_ptr<IVertexBuffer> CreateVertexBuffer(VertexBufferType type, const BufferTypeInfo& dataType, size_t dataLength, const void* data) const = 0;
    [[nodiscard]] virtual std::shared_ptr<IShaderProgram> CreateShaderProgramFromFiles(std::initializer_list<str> files) const = 0;

	virtual void ReloadResources(ReloadableGroup group) = 0;
};

class IRenderer
{
public:
	NON_COPYABLE_OR_MOVABLE(IRenderer)

	IRenderer() = default;
	virtual ~IRenderer() = default;

public:
    [[nodiscard]] virtual IResourceFactory& GetResourceFactory() const = 0;
    [[nodiscard]] virtual IStateManager& GetStateManager() const = 0;
    [[nodiscard]] virtual IRendererCapabilities& GetRendererCapabilities() const = 0;

	virtual void Shutdown() = 0;

	virtual void DispatchCompute(glm::uvec3 threadGroupSize) = 0;
	//Draws count vertices connected by primitive type.
	virtual void Draw(Primitives::Primitive type, long int first, long int count, int numInstances = 1, int baseVertex = 0) = 0;

	virtual void SetViewport(const AABB2d& viewport) = 0;
	virtual void ClearBuffer(const glm::vec4& color) = 0;
	virtual void ClearDepth(float depth) = 0;
	virtual void FinishFrame() = 0;

    [[nodiscard]] virtual IFrameBuffer& GetDefaultFramebuffer() const = 0;
};


using TextureRef = std::shared_ptr<ITexture>;

}

#endif
