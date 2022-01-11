#ifndef AT2_MAIN_HEADER_H
#define AT2_MAIN_HEADER_H

//#define GLM_FORCE_SWIZZLE
#include <glm/gtx/vec_swizzle.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <array>
#include <memory>
#include <span>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "log.h"
#include "utils.hpp"

#include "AT2_states.hpp"
#include "AT2_textures.hpp"
#include "AT2_types.hpp"
#include "AT2_exceptions.hpp"

#define NON_COPYABLE_OR_MOVABLE(type)                                                                                  \
    type(const type&) = delete;                                                                                        \
    type(type&&) = delete;                                                                                             \
    type& operator=(const type&) = delete;                                                                             \
    type& operator=(type&&) = delete;

namespace AT2
{

    typedef std::string str;

    class IReloadable;
    class IBuffer;
    class IFrameBuffer;
    class IVertexArray;
    class ITexture;
    class IShaderProgram;
    class IUniformContainer;
    class IRendererCapabilities;
    class IStateManager;
    class IResourceFactory;
    class IRenderer;

    class ITime
    {
    public:
        virtual ~ITime() = default;

    public:
        [[nodiscard]] virtual Seconds getTime() const = 0;
        [[nodiscard]] virtual Seconds getDeltaTime() const = 0;
    };

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
        IBuffer() = default;
        virtual ~IBuffer() = default;

    public:
        template <std::ranges::contiguous_range T>
        void SetData(const T& data)
        {
            SetDataRaw(std::as_bytes(std::span {data}));
        }

        [[nodiscard]] virtual size_t GetLength() const noexcept = 0;
        virtual void SetDataRaw(std::span<const std::byte> data) = 0;

        virtual std::span<std::byte> Map(BufferUsage usage) = 0;
        virtual std::span<std::byte> MapRange(BufferUsage usage, size_t offset, size_t length) = 0;
        virtual void Unmap() = 0;


        //[[nodiscard]] virtual unsigned int GetId() const noexcept = 0;
        //[[nodiscard]] virtual VertexBufferType GetType() const noexcept = 0;
    protected:
    };

    //TODO: evolve to IRenderPass
    class IFrameBuffer
    {
    public:
        NON_COPYABLE_OR_MOVABLE(IFrameBuffer)

        IFrameBuffer() = default;
        virtual ~IFrameBuffer() = default;
        
        struct ColorAttachment
        {
            ColorAttachment(std::shared_ptr<ITexture> texture, std::optional<glm::vec4> clearColor = {}) :
                Texture {texture}, ClearColor {clearColor}
            {}

            std::shared_ptr<ITexture> Texture;
            std::optional<glm::vec4> ClearColor;
        };

        struct DepthAttachment
        {
            DepthAttachment(std::shared_ptr<ITexture> texture = nullptr, std::optional<double> clearDepth = {}) :
                Texture {texture}, ClearDepth {clearDepth}
            {}

            std::shared_ptr<ITexture> Texture;
            std::optional<double> ClearDepth;
        };

        using RenderFunc = std::function<void(IRenderer&)>;

    public:
        //TODO: the Builder pattern, or encapsulate params in special descriptor class?

        virtual void SetColorAttachment(unsigned int attachmentNumber, ColorAttachment attachment) = 0;
        [[nodiscard]] virtual const ColorAttachment* GetColorAttachment(unsigned int attachmentNumber) const = 0;
        virtual void SetDepthAttachment(DepthAttachment attachment) = 0;
        [[nodiscard]] virtual const DepthAttachment* GetDepthAttachment() const = 0;

        // set clear color for all attachments
        virtual void SetClearColor(std::optional<glm::vec4> color) = 0;
        virtual void SetClearDepth(std::optional<float> depth) = 0;

        //TODO: support stencil attachment

        [[nodiscard]] virtual glm::ivec2 GetActualSize() const = 0;

        virtual void Render(RenderFunc renderFunc) = 0;
    };

    class IVertexArray
    {
    public:
        NON_COPYABLE_OR_MOVABLE(IVertexArray)

        IVertexArray() = default;
        virtual ~IVertexArray() = default;

    public:
        [[nodiscard]] virtual unsigned int GetId() const noexcept = 0;

        virtual void SetIndexBuffer(std::shared_ptr<IBuffer> buffer, BufferDataType type) = 0;
        [[nodiscard]] virtual std::shared_ptr<IBuffer> GetIndexBuffer() const = 0;
        [[nodiscard]] virtual std::optional<BufferDataType> GetIndexBufferType() const = 0;

        //TODO GetOrCreateVertexBuffer ?
        virtual void SetAttributeBinding(unsigned int attributeIndex, std::shared_ptr<IBuffer> buffer,
                                     const BufferBindingParams& bindingParams) = 0;
        [[nodiscard]] virtual std::shared_ptr<IBuffer> GetVertexBuffer(unsigned int index) const = 0;
        [[nodiscard]] virtual std::optional<BufferBindingParams> GetVertexBufferBinding(unsigned int index) const = 0;
    };


    //Could be just a read-only tuple of params, OOP interface is just for unification
    class ISampler
    {
    public:
        ISampler() = default;
        virtual ~ISampler() = default;

    public:
        virtual void SetWrapMode(TextureWrapParams wrapParams) = 0;
        [[nodiscard]] virtual const TextureWrapParams& GetWrapMode() const noexcept = 0;

        virtual void SetSamplingMode(TextureSamplingParams samplingParams) = 0;
        [[nodiscard]] virtual const TextureSamplingParams& GetSamplingParams() const noexcept = 0;

        virtual void SetAnisotropy(float anisotropy) = 0;
        [[nodiscard]] virtual float GetAnisotropy() const noexcept = 0;
    };

    class ITexture : public ISampler
    {
    public:
        NON_COPYABLE_OR_MOVABLE(ITexture)

        ITexture() = default;
        virtual ~ITexture() override = default;

    public:
        //TODO: think about interface
        virtual void BindAsImage(unsigned int unit, glm::u32 level, glm::u32 layer, bool isLayered,
                                 BufferUsage usage = BufferUsage::ReadWrite) const = 0;
        virtual void BuildMipmaps() = 0;

        [[nodiscard]] virtual unsigned int GetId() const noexcept = 0;
        [[nodiscard]] virtual glm::uvec3 GetSize() const noexcept = 0;
        [[nodiscard]] virtual size_t GetDataLength() const noexcept = 0;

        [[nodiscard]] virtual const Texture& GetType() const noexcept = 0;

        //TODO: think how to make better
        // Set data of a Texture1d
        virtual void SubImage1D(glm::u32 offset, glm::u32 size, glm::u32 level, ExternalTextureFormat dataFormat,
                                const void* data) = 0;
        // Set data of a Texture1DArray and Texture2D
        virtual void SubImage2D(glm::uvec2 offset, glm::uvec2 size, glm::u32 level, ExternalTextureFormat dataFormat,
                                const void* data) = 0;
        // Set data of a Texture2DArray, Texture3D, TextureCubeArray, TextureCube
        virtual void SubImage3D(glm::uvec3 offset, glm::uvec3 size, glm::u32 level, ExternalTextureFormat dataFormat,
                                const void* data) = 0;
    };

    class IShaderProgram
    {
    public:
        NON_COPYABLE_OR_MOVABLE(IShaderProgram)

        IShaderProgram() = default;
        virtual ~IShaderProgram() = default;

    public:
        virtual void Bind() = 0;
        [[nodiscard]] virtual unsigned int GetId() const noexcept = 0;
        [[nodiscard]] virtual bool IsActive() const noexcept = 0;

        virtual std::unique_ptr<IUniformContainer> CreateAssociatedUniformStorage(std::string_view blockName = "") = 0;
        virtual void AttachShader( std::string_view code, ShaderType type) = 0;

        //Warning: Shader reloading/relinking will invalidate that state
        //TODO: make uniform buffers binding same as textures binding via StateManager
        virtual void SetUBO(std::string_view blockName, unsigned int index) = 0;
        virtual void SetUniform(std::string_view name, Uniform value) = 0;
        virtual void SetUniformArray(std::string_view name, UniformArray value) = 0;
    };

    class IUniformContainer
    {
    public:
        NON_COPYABLE_OR_MOVABLE(IUniformContainer)

        IUniformContainer() = default;
        virtual ~IUniformContainer() = default;

    public:
        virtual void SetUniform(std::string_view name, const Uniform& value) = 0;
        virtual void SetUniform(std::string_view name, const std::shared_ptr<ITexture>& value) = 0;

        virtual void Bind(IStateManager& stateManager) const = 0;
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

    class IStateManager
    {
    public:
        NON_COPYABLE_OR_MOVABLE(IStateManager)

        IStateManager() = default;
        virtual ~IStateManager() = default;

    public:
        virtual void BindTextures(const TextureSet& textures) = 0; //TODO: more flexible interface with possibility to add textures one-by-one + something like UnbindTextures() 
        virtual void BindShader(const std::shared_ptr<IShaderProgram>& shader) = 0;
        virtual void BindVertexArray(const std::shared_ptr<IVertexArray>& vertexArray) = 0;

        virtual void ApplyState(RenderState state) = 0;

        //virtual TextureSet& GetActiveTextures() const = 0;
        //[[nodiscard]] virtual std::shared_ptr<IFrameBuffer> GetActiveFrameBuffer() const = 0;
        [[nodiscard]] virtual std::shared_ptr<IShaderProgram> GetActiveShader() const = 0;
        [[nodiscard]] virtual std::shared_ptr<IVertexArray> GetActiveVertexArray() const = 0;

        [[nodiscard]] virtual std::optional<BufferDataType> GetIndexDataType() const noexcept = 0;

    	[[nodiscard]] virtual std::optional<unsigned int> GetActiveTextureIndex(std::shared_ptr<const ITexture> texture) const noexcept = 0;
    };

    class IResourceFactory
    {
    public:
        NON_COPYABLE_OR_MOVABLE(IResourceFactory)

        IResourceFactory() = default;
        virtual ~IResourceFactory() = default;

    public:
        template <std::ranges::contiguous_range T>
        std::shared_ptr<IBuffer> MakeBufferFrom(VertexBufferType type, const T& data)
        {
            return CreateBuffer(type, std::as_bytes(std::span {data}));
        }

    public:
        [[nodiscard]] virtual std::shared_ptr<ITexture> CreateTextureFromFramebuffer(const glm::ivec2& pos,
                                                                                     const glm::uvec2& size) const = 0;
        [[nodiscard]] virtual std::shared_ptr<ITexture> CreateTexture(const Texture& declaration,
                                                                      ExternalTextureFormat desiredFormat) const = 0;
        [[nodiscard]] virtual std::shared_ptr<IFrameBuffer> CreateFrameBuffer() const = 0;
        [[nodiscard]] virtual std::shared_ptr<IVertexArray> CreateVertexArray() const = 0;

        [[nodiscard]] virtual std::shared_ptr<IBuffer> CreateBuffer(VertexBufferType type) const = 0;
        [[nodiscard]] virtual std::shared_ptr<IBuffer> CreateBuffer(VertexBufferType type, std::span<const std::byte> data) const = 0;
        [[nodiscard]] virtual std::shared_ptr<IShaderProgram>
            CreateShaderProgramFromFiles(std::initializer_list<str> files) const = 0;

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

        virtual void DispatchCompute(glm::uvec3 threadGroupSize) = 0;
        //Draws count vertices connected by primitive type.
        virtual void Draw(Primitives::Primitive type, size_t first, long int count, int numInstances = 1, int baseVertex = 0) = 0;

        virtual void SetViewport(const AABB2d& viewport) = 0; //TODO: move out
        virtual void BeginFrame() = 0;
        virtual void FinishFrame() = 0;

        [[nodiscard]] virtual IFrameBuffer& GetDefaultFramebuffer() const = 0;
    };


    using TextureRef = std::shared_ptr<ITexture>;

    //TODO: move from main header
    template <typename... Args>
    requires(std::ranges::contiguous_range<Args> &&...)
    [[nodiscard]] std::shared_ptr<IVertexArray> MakeVertexArray(IResourceFactory& factory,
                                                                std::pair<unsigned, const Args&>... args)
    {
        auto vertexArray = factory.CreateVertexArray();
        static_cast<void>(std::initializer_list<int> {
            (vertexArray->SetAttributeBinding(args.first,
                                              factory.MakeBufferFrom(VertexBufferType::ArrayBuffer, args.second),
                                              BufferDataTypes::BufferTypeOf<std::remove_cvref_t<decltype(args.second[0])>>),
             0)...});

        return vertexArray;
    }

} // namespace AT2

#endif
