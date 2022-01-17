#pragma once

#include <map>
#include "AT2lowlevel.h"

namespace AT2::Metal
{
    class Renderer;

    class ShaderLibrary final
    {
    public:
        //using LazyFunctionGetter = std::function<MtlPtr<MTL::Function>()>;
        using FunctionSpecializeErrorHandler = std::function<void(NS::Error*)>;
        
        ShaderLibrary(Renderer& renderer, std::string_view source);
        
        //TODO: make asynchonous and caching, if it could be useful
        MtlPtr<MTL::Function> GetOrCreateFunction(std::string_view name, MtlPtr<MTL::FunctionConstantValues> constantValues = nullptr, const FunctionSpecializeErrorHandler& errorHandle = {} );
        
        void SetLabel(std::string_view name);
        std::string_view GetLabel() const;
        std::string_view GetName() const;
        
        const MtlPtr<MTL::Library>& GetLibrary() { return m_library; }
        Renderer& GetVisualizationSystem() const { return m_renderer; }
        
    private:
        Renderer& m_renderer;
        MtlPtr<MTL::Library> m_library;
    };

    class ShaderProgram : public IShaderProgram//, public std::enable_shared_from_this<ShaderProgram>
    {
    public:
        NON_COPYABLE_OR_MOVABLE(ShaderProgram)

        struct Descriptor
        {
            std::shared_ptr<ShaderLibrary> Library;
            std::string VertexFunc;
            std::string FragmentFunc;
        };
        
        ShaderProgram( const Descriptor& descriptor );
        ~ShaderProgram() override;

    public:
        std::unique_ptr<IUniformContainer> CreateAssociatedUniformStorage(std::string_view blockName) override;

        void SetUBO(std::string_view blockName, unsigned int index) override;
        void SetUniform(std::string_view name, Uniform value) override;
        void SetUniformArray(std::string_view name, UniformArray value) override;

        virtual const str& GetName() { return m_name; }
        virtual void SetName(const str& name) { m_name = name; }

    // for internal usage
        MtlPtr<MTL::Library> GetLibrary() { return m_library->GetLibrary(); }
        void Apply(MTL::RenderPipelineDescriptor& pipelineDescriptor) const;
        void OnStateCreated(MTL::RenderPipelineReflection* reflection);
        
    private:
        std::shared_ptr<ShaderLibrary> m_library;
        MtlPtr<MTL::Function> m_functionVertex, m_functionFragment;
        
        str m_name;
    };
} // namespace AT2::Metal
