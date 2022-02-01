#include "ShaderProgram.h"
#include <DataLayout/StructuredBuffer.h>
#include "Renderer.h"

using namespace AT2;
using namespace AT2::Metal;

namespace {

}


ShaderLibrary::ShaderLibrary(Renderer& renderer, std::string_view source)
: m_renderer{renderer}
{
    auto sourceString = MakeMetalString(source);
    assert(sourceString.use_count() == 1); // SUDDENOUS "unit" test about reference counting
    
    const auto compileOptions = ConstructMetalObject<MTL::CompileOptions>();
    //compileOptions->setLibraryType(MTL::LibraryTypeDynamic);
    //compileOptions->setLanguageVersion(MTL::LanguageVersion2_0);
    
    NS::Error* errorOutput = nullptr;
    m_library = renderer.getDevice()->newLibrary(sourceString.get(), compileOptions.get(), &errorOutput);
    CheckErrors(errorOutput);
}

MtlPtr<MTL::Function> ShaderLibrary::GetOrCreateFunction(std::string_view name, MtlPtr<MTL::FunctionConstantValues> constantValues, const FunctionSpecializeErrorHandler& errorHandler)
{
    auto getFunction = [this, &constantValues, &errorHandler, name = MakeMetalString(name)](){
        NS::Error* error = nullptr;
        Utils::final_action { [error, &errorHandler]{ if (error && errorHandler) errorHandler(error); }};
        
        if (constantValues)
            return m_library->newFunction(name.get(), constantValues.get(), &error);
        
        return m_library->newFunction(name.get());
    };
    
    return getFunction();
}

void ShaderLibrary::SetLabel(std::string_view name)
{
    m_library->setLabel(MakeMetalString(name).release());
}

std::string_view ShaderLibrary::GetLabel() const
{
    return m_library->label()->cString(NS::UTF8StringEncoding);
}

std::string_view ShaderLibrary::GetName() const
{
    return m_library->label()->cString(NS::UTF8StringEncoding);
}


ShaderProgram::ShaderProgram(const Descriptor& descriptor)
: m_library(descriptor.Library)
{
    if (!m_library)
        throw AT2ShaderException("Metal::ShaderProgram: no library specified");    

    m_functionVertex = m_library->GetOrCreateFunction(descriptor.VertexFunc);
    m_functionFragment = m_library->GetOrCreateFunction(descriptor.FragmentFunc);
}

ShaderProgram::~ShaderProgram()
{
	
}

std::unique_ptr<StructuredBuffer> ShaderProgram::CreateAssociatedUniformStorage(std::string_view blockName)
{
    if (!m_introspection)
        return nullptr;
    
    auto buffer = m_library->GetVisualizationSystem().GetResourceFactory().CreateBuffer(VertexBufferType::UniformBuffer);
    
    //TODO: handle multiple layouts?
    std::vector<BufferLayout> layouts;
    m_introspection->FindBuffer(blockName, [&](const Introspection::BufferInfo& bufferInfo){
        layouts.push_back(bufferInfo.Layout);
    });
    
    if (layouts.empty())
        return nullptr;
        
    return std::make_unique<StructuredBuffer>(std::move(buffer), std::make_shared<BufferLayout>(std::move(layouts.front())));
}

void ShaderProgram::Apply(MTL::RenderPipelineDescriptor& pipelineDescriptor) const
{
    pipelineDescriptor.setVertexFunction(m_functionVertex.get());
    pipelineDescriptor.setFragmentFunction(m_functionFragment.get());
}

void ShaderProgram::OnStateCreated(MtlPtr<MTL::RenderPipelineReflection> reflection)
{
    m_introspection = std::make_shared<Introspection::ProgramIntrospection>(std::move(reflection));
    m_defaultUniformBlock = CreateAssociatedUniformStorage(DefaultUniformBlockName);
}
