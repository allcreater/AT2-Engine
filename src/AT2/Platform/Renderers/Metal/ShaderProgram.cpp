#include "ShaderProgram.h"
#include "UniformBuffer.h"
#include "Renderer.h"

using namespace AT2;
using namespace AT2::Metal;

namespace {

}


ShaderProgram::ShaderProgram(Renderer& renderer) : m_renderer(renderer)
{
constexpr char source[] = R"(
    #include <metal_stdlib>
    using namespace metal;

    struct VertexIn
    {
        float3 position [[ attribute(1) ]];
        float2 texCoord [[ attribute(2) ]];
    };

    struct VertexUniforms
    {
        float4x4 u_matModelView [[id(0)]];
        float4x4 u_matProjection;
    };

    vertex float4 vertex_main(
        const VertexIn vertex_in        [[ stage_in ]],
        constant VertexUniforms& params [[ buffer(0) ]]
    )
    {
        const auto viewSpacePos = params.u_matModelView * float4(vertex_in.position, 1);
        return params.u_matProjection * viewSpacePos;
    }

    fragment float4 fragment_main(texture2d<float, access::sample> texAlbedo [[texture(0)]])
    {
        return float4(1, 0, 0, 1);
    }
)";
    
    auto sourceString = NS::String::string(source, NS::UTF8StringEncoding);
    
    const auto compileOptions = ConstructMetalObject<MTL::CompileOptions>();
    //compileOptions->setLibraryType(MTL::LibraryTypeDynamic);
    //compileOptions->setLanguageVersion(MTL::LanguageVersion2_0);
    
    NS::Error* errorOutput = nullptr;
    m_library = renderer.getDevice()->newLibrary(sourceString, compileOptions.get(), &errorOutput);
    CheckErrors(errorOutput);
}

ShaderProgram::~ShaderProgram()
{
	
}

std::unique_ptr<IUniformContainer> ShaderProgram::CreateAssociatedUniformStorage(std::string_view blockName)
{
    return std::make_unique<UniformBuffer>(m_renderer);
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
