#pragma once

#include "AT2lowlevel.h"
#include <DataLayout/BufferLayout.h>

namespace AT2::Metal::Introspection
{

enum class ShaderType
{
    Vertex      = 0,
    Fragment    = 1,
    Tile        = 2
};

struct ArgumentInfo
{
    ShaderType Shader;
    unsigned int BindingIndex;
};

struct BufferInfo : ArgumentInfo
{
    BufferLayout Layout;
};

class ProgramIntrospection
{
public:
    ProgramIntrospection(MtlPtr<MTL::RenderPipelineReflection> reflection);
    
    //TODO: generator coroutine
    void FindTexture(std::string_view name, std::function<void(const ArgumentInfo& info)> callback) const;
    void FindBuffer(std::string_view name, std::function<void(const BufferInfo& info)> callback) const ;
    
private:
    void VisitArgument(ShaderType shaderType, const MTL::Argument* argument);
    
private:
    MtlPtr<MTL::RenderPipelineReflection> m_reflection;
    std::unordered_multimap<std::string_view, BufferInfo> m_inputBuffers;
    std::unordered_multimap<std::string_view, ArgumentInfo> m_inputTextures;
};

}
