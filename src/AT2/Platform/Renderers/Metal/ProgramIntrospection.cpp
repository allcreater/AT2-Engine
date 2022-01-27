#include "ProgramIntrospection.h"

using namespace AT2;
using namespace Metal;
using namespace Introspection;

namespace
{
std::pair<unsigned int, unsigned int> GetMatrixSizeAndAStride(MTL::DataType dataType)
{
    switch (dataType)
    {
        case MTL::DataTypeHalf2x2: return {8, 4};
        case MTL::DataTypeHalf2x3: return {16, 8};
        case MTL::DataTypeHalf2x4: return {16, 8};
        case MTL::DataTypeHalf3x2: return {12, 4};
        case MTL::DataTypeHalf3x3: return {24, 8};
        case MTL::DataTypeHalf3x4: return {24, 8};
        case MTL::DataTypeHalf4x2: return {16, 4};
        case MTL::DataTypeHalf4x3: return {32, 8};
        case MTL::DataTypeHalf4x4: return {32, 8};
            
        case MTL::DataTypeFloat2x2: return {16, 8};
        case MTL::DataTypeFloat2x3: return {32, 16};
        case MTL::DataTypeFloat2x4: return {32, 16};
        case MTL::DataTypeFloat3x2: return {24, 8};
        case MTL::DataTypeFloat3x3: return {48, 16};
        case MTL::DataTypeFloat3x4: return {48, 16};
        case MTL::DataTypeFloat4x2: return {32, 8};
        case MTL::DataTypeFloat4x3: return {64, 16};
        case MTL::DataTypeFloat4x4: return {64, 16};
        default: //not matrix
            return {0, 0};
    }
}

}


ProgramIntrospection::ProgramIntrospection(MtlPtr<MTL::RenderPipelineReflection> reflection)
: m_reflection{std::move(reflection)}
{
    VisitArray<const MTL::Argument*>(m_reflection->vertexArguments(), [this](const MTL::Argument* argument){ VisitArgument(ShaderType::Vertex, argument);});
    VisitArray<const MTL::Argument*>(m_reflection->fragmentArguments(), [this](const MTL::Argument* argument){ VisitArgument(ShaderType::Fragment, argument);});
    VisitArray<const MTL::Argument*>(m_reflection->tileArguments(), [this](const MTL::Argument* argument){ VisitArgument(ShaderType::Tile, argument);});
}

void ProgramIntrospection::VisitArgument(ShaderType shaderType, const MTL::Argument* argument)
{
    if (!argument)
        return;
    
    auto argumentName = argument->name()->cString(NS::UTF8StringEncoding);
    
    switch (argument->type())
    {
        case MTL::ArgumentTypeBuffer:
        {

            std::vector<Field> knownFields;
            VisitArray<MTL::StructMember*>(argument->bufferStructType()->members(), [&knownFields](MTL::StructMember* member){
                auto name = member->name()->cString(NS::UTF8StringEncoding);
                auto dataType = member->dataType();
                
                auto arrayAttribs = [&]{
                    if (dataType != MTL::DataTypeArray)
                        return ArrayAttributes{};
                    
                    return ArrayAttributes{
                        static_cast<unsigned int>(member->arrayType()->arrayLength()),
                        static_cast<unsigned int>(member->arrayType()->stride())};
                }();
                
                auto [matrixSize, matrixStride] = GetMatrixSizeAndAStride(member->dataType()); //zeroes if not matrix, fine too
                knownFields.emplace_back(name, member->offset(), arrayAttribs, matrixStride, matrixSize );
            });
            
            m_inputBuffers.emplace(argumentName, BufferInfo{shaderType, static_cast<unsigned int>(argument->index()), BufferLayout{std::move(knownFields)}});
        } break;
            
        case MTL::ArgumentTypeTexture:
        {
            m_inputTextures.emplace(argumentName, ArgumentInfo{shaderType, static_cast<unsigned int>(argument->index())});
            
            auto type = argument->textureType();
            auto argumentIndex = argument->index();
            auto texType = argument->textureDataType();
        } break;
    };
}

void ProgramIntrospection::FindTexture(std::string_view name, std::function<void(const ArgumentInfo& info)> callback) const
{
    auto [begin, end] = m_inputTextures.equal_range(name);
    for (auto it = begin; it != end; ++it)
    {
        callback(it->second);
    }
}

void ProgramIntrospection::FindBuffer(std::string_view name,std::function<void(const BufferInfo& info)> callback) const
{
    auto [begin, end] = m_inputBuffers.equal_range(name);
    for (auto it = begin; it != end; ++it)
    {
        callback(it->second);
    }
}
