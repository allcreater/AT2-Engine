#include "GlProgramIntrospection.h"

namespace
{
    template <size_t N>
    std::array<GLint, N> getProgramResource(GLuint program, GLenum interface, GLuint index,
                                            const std::array<GLenum, N>& request)
    {
        auto result = std::array<GLint, N> {};
        glGetProgramResourceiv(program, interface, index, request.size(), request.data(), result.size(), nullptr,
                               result.data());

        return result;
    }
}; // namespace

using namespace AT2::OpenGl::Introspection;

std::unique_ptr<ProgramInfo> ProgramInfo::Request(GLuint program)
{
    GLint numUniforms = 0;
    glGetProgramInterfaceiv(program, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms);

    constexpr std::array<GLenum, 7> uniformAttributes {
        GL_NAME_LENGTH, GL_LOCATION, GL_TYPE, GL_BLOCK_INDEX, GL_OFFSET, GL_ARRAY_STRIDE, GL_MATRIX_STRIDE
    };

    auto programInfo = std::unique_ptr<ProgramInfo> { new ProgramInfo() };

    for (GLuint uniformIndex = 0; uniformIndex < static_cast<GLuint>(numUniforms); ++uniformIndex)
    {
        const auto [nameLength, location, type, blockIndex, offset, arrayStride, matrixStride] =
            getProgramResource(program, GL_UNIFORM, uniformIndex, uniformAttributes);

        //ask a uniform's name
        std::string uniformName(nameLength-1, '\0');  //because nameLength includes \0
        glGetProgramResourceName(program, GL_UNIFORM, uniformIndex, uniformName.size()+1, nullptr, uniformName.data());

        if (blockIndex == static_cast<GLint>(GL_INVALID_INDEX)) //it's free uniforms
        {
            programInfo->uniforms.try_emplace(std::move(uniformName), UniformInfo{location, type});
        }
        else //it's uniform from uniform block
        {
            const auto& [iterator, firstTime] = programInfo->uniformBlocks.try_emplace(blockIndex);
            auto& block = iterator->second;

            if (firstTime)
            {
                const auto [nameLength, dataSize, binding] = getProgramResource(program, GL_UNIFORM_BLOCK, blockIndex,
                                       std::array<GLenum, 3> {GL_NAME_LENGTH, GL_BUFFER_DATA_SIZE, GL_BUFFER_BINDING});

                std::string blockName(nameLength-1, '\0'); //because nameLength includes \0
                glGetProgramResourceName(program, GL_UNIFORM_BLOCK, blockIndex, blockName.size()+1, nullptr,
                                         blockName.data());

                block.Name = std::move(blockName);
                block.InitialBinding = binding;
                block.BlockIndex = blockIndex;
                block.DataSize = dataSize;

                //add it to lookup
                programInfo->uniformBlocksByName.try_emplace(block.Name, &block);
            }

            
            block.Uniforms.try_emplace(std::move(uniformName), BufferedUniformInfo{{location, type}, offset, arrayStride, matrixStride});
        }
    }

    return programInfo;
}

const UniformBlockInfo* ProgramInfo::getUniformBlock(GLint blockIndex) const
{
    return Utils::find(uniformBlocks, blockIndex);
}

const UniformBlockInfo* ProgramInfo::getUniformBlock(std::string_view blockName) const
{
    return Utils::find(uniformBlocksByName, blockName);
}
