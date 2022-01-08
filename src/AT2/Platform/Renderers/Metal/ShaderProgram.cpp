#include "ShaderProgram.h"
#include "UniformBuffer.h"
#include "Renderer.h"

using namespace AT2;
using namespace AT2::Metal;

namespace {

}


ShaderProgram::ShaderProgram(Renderer& renderer) : renderer(renderer)
{
constexpr char source[] = R"(
    #include <metal_stdlib>
    using namespace metal;

    struct VertexIn {
      float4 position [[ attribute(0) ]];
    };

    vertex float4 vertex_main(const VertexIn vertex_in [[ stage_in ]]) {
      return vertex_in.position;
    }

    fragment float4 fragment_main() {
      return float4(1, 0, 0, 1);
    }
)";
    
    auto checkErrors = [](NS::Error* error){
        if (!error)
            return;
        
        const auto* description = error->description()->cString(NS::UTF8StringEncoding);
        throw AT2ShaderException( description );
    };
    
    auto sourceString = NS::String::string(source, NS::UTF8StringEncoding);
    
    const auto compileOptions = ConstructMetalObject<MTL::CompileOptions>();
    //compileOptions->setLibraryType(MTL::LibraryTypeDynamic);
    //compileOptions->setLanguageVersion(MTL::LanguageVersion2_0);
    
    NS::Error* errorOutput = nullptr;
    library = renderer.getDevice()->newLibrary(sourceString, compileOptions.get(), &errorOutput);
    checkErrors(errorOutput);
    
    
    auto* funcVS = library->newFunction(NS::String::string("vertex_main", NS::ASCIIStringEncoding));
    auto* funcFS = library->newFunction(NS::String::string("fragment_main", NS::ASCIIStringEncoding));
    
    const auto vd = ConstructMetalObject<MTL::VertexDescriptor>();
    //vd->attributes()->object(0)->
    
    const auto pd = ConstructMetalObject<MTL::RenderPipelineDescriptor>();
    pd->setVertexFunction(funcVS);
    pd->setFragmentFunction(funcFS);
    pd->setVertexDescriptor(vd.get());
    
    //pipeline = renderer.getDevice()->newRenderPipelineState(pd.get(), &errorOutput);

    checkErrors(errorOutput);
    
}

ShaderProgram::~ShaderProgram()
{
	
}

void ShaderProgram::Bind()
{
    assert (renderer.getFrameContext());
    
    //renderer.getFrameContext()->renderEncoder->setRenderPipelineState(pipeline.get());
}

bool ShaderProgram::IsActive() const noexcept
{
    return false;
}

std::unique_ptr<IUniformContainer> ShaderProgram::CreateAssociatedUniformStorage(std::string_view blockName)
{
    return std::make_unique<UniformBuffer>(renderer);
}

void ShaderProgram::AttachShader(std::string_view data, ShaderType type)
{
    /*
    std::string dataCopy;
    auto sourceString = NS::String::string(dataCopy.data(), NS::UTF8StringEncoding); // doing copy twice, bad =(
     */
}

void ShaderProgram::SetUBO(std::string_view blockName, unsigned int index)
{
	
}

void ShaderProgram::SetUniform(std::string_view name, Uniform value)
{
	
}

void ShaderProgram::SetUniformArray(std::string_view name, UniformArray value)
{
	
}

bool ShaderProgram::TryCompile()
{
    return false;
}

void ShaderProgram::CleanUp() {}
