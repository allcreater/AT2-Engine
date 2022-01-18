#include "ShaderProgram.h"
#include "UniformBuffer.h"
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

std::unique_ptr<IUniformContainer> ShaderProgram::CreateAssociatedUniformStorage(std::string_view blockName)
{
    return std::make_unique<UniformBuffer>(m_library->GetVisualizationSystem());
}


void ShaderProgram::SetUBO(std::string_view blockName, unsigned int index)
{
	
}

void ShaderProgram::SetUniform(std::string_view name, Uniform value)
{
    using namespace std::literals;

    const auto* mat = std::get_if<glm::mat4>(&value);
    if (!mat)
        return;
    
    if (name == "u_matModelView"sv)
        m_crutchBuffer.u_matModelView = *mat;
    else if (name == "u_matProjection"sv)
        m_crutchBuffer.u_matProjection = *mat;
    
    
    //m_reflection->vertexArguments();
}

void ShaderProgram::SetUniformArray(std::string_view name, UniformArray value)
{
	
}

void ShaderProgram::Apply(MTL::RenderPipelineDescriptor& pipelineDescriptor) const
{
    pipelineDescriptor.setVertexFunction(m_functionVertex.get());
    pipelineDescriptor.setFragmentFunction(m_functionFragment.get());
}

namespace
{
    void VisitArgument(const MTL::Argument* argument)
    {
        if (!argument)
            return;
        
        auto argumentName = argument->name()->cString(NS::ASCIIStringEncoding);
        
        switch (argument->type())
        {
            case MTL::ArgumentTypeBuffer:
            {
                VisitArray<MTL::StructMember*>(argument->bufferStructType()->members(), [](const MTL::StructMember* member){
                    auto name = member->name()->cString(NS::UTF8StringEncoding);
                    auto dataType = member->dataType();
                });
            } break;
                
            case MTL::ArgumentTypeTexture:
            {
                
            } break;
        };
    }

    void VisitArgumentArray(const NS::Array* argumentsArray)
    {
        VisitArray<const MTL::Argument*>(argumentsArray, VisitArgument);
    }

}

void ShaderProgram::OnStateCreated(MtlPtr<MTL::RenderPipelineReflection> reflection)
{
    VisitArgumentArray(reflection->vertexArguments());
    VisitArgumentArray(reflection->fragmentArguments());
    
    m_reflection = std::move(reflection);
}

void ShaderProgram::OnDrawCall(MTL::RenderCommandEncoder* renderEncoder)
{
    renderEncoder->setVertexBytes(&m_crutchBuffer, sizeof(m_crutchBuffer), 0);
}
