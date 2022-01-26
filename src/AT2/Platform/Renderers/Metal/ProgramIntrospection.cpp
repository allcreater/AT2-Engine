#include "ProgramIntrospection.h"

using namespace AT2;
using namespace Metal;
using namespace Introspection;

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
                
                knownFields.emplace_back(name, member->offset(), arrayAttribs, 0 );
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
