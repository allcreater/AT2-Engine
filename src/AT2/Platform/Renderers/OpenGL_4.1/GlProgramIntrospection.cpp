#include "GlProgramIntrospection.h"

using namespace std::literals;

using namespace AT2;
using namespace OpenGL41::Introspection;

namespace
{
    std::vector<GLint> getActiveUniformsParams(GLuint program, std::span<GLuint> uniformIndices, GLenum paramName)
    {
        assert(uniformIndices.size() < static_cast<size_t>(std::numeric_limits<GLsizei>::max()));

        std::vector<GLint> params(uniformIndices.size());
        glGetActiveUniformsiv(program, static_cast<GLsizei>(uniformIndices.size()), uniformIndices.data(), paramName,
                            params.data());
        return params;
    }

    std::string getUniformBlockName(GLuint program, const GLuint blockIndex)
    {
        GLsizei blockNameLength;
        glGetActiveUniformBlockiv(program, blockIndex, GL_UNIFORM_BLOCK_NAME_LENGTH, &blockNameLength);

        std::string name(blockNameLength, '\0');
        glGetActiveUniformBlockName(program, blockIndex, blockNameLength, &blockNameLength, name.data());
        assert(name.size() == blockNameLength + 1);
        name.resize(blockNameLength);

        return name;
    }

    UniformBlockInfo getUniformBlockInfo(const GLuint program, const GLuint blockIndex, std::span<char> uniformNameBuffer)
    {
        GLint blockDataSize, blockNumActiveUniforms, blockBinding;
        glGetActiveUniformBlockiv(program, blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockDataSize);
        glGetActiveUniformBlockiv(program, blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &blockNumActiveUniforms);
        glGetActiveUniformBlockiv(program, blockIndex, GL_UNIFORM_BLOCK_BINDING, &blockBinding);
        
        //first we get active uniforms indices
        std::vector<GLuint> activeUniformIndices(blockNumActiveUniforms);
        glGetActiveUniformBlockiv(program, blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES,
                                reinterpret_cast<GLint*>(activeUniformIndices.data()));

        //and then...
        std::vector<GLint> activeUniformOffsets = getActiveUniformsParams(program, activeUniformIndices, GL_UNIFORM_OFFSET);
        //std::vector<GLint> activeUniformTypes = getActiveUniformsParams(program, activeUniformIndices, GL_UNIFORM_TYPE);
        std::vector<GLint> activeUniformArraySizes = getActiveUniformsParams(program, activeUniformIndices, GL_UNIFORM_SIZE);
        std::vector<GLint> activeUniformArrayStrides = getActiveUniformsParams(program, activeUniformIndices, GL_UNIFORM_ARRAY_STRIDE);
        std::vector<GLint> activeUniformMatrixStrides = getActiveUniformsParams(program, activeUniformIndices, GL_UNIFORM_MATRIX_STRIDE);
        //std::vector<GLint> activeUniformMatrixIsRowMajor = getActiveUniformsParams(program, activeUniformIndices, GL_UNIFORM_IS_ROW_MAJOR);


        std::vector<Field> bufferFields;
        bufferFields.reserve(blockNumActiveUniforms);

        //read uniform names and collect all data into layout
        for (int i = 0; i < blockNumActiveUniforms; ++i)
        {
            GLsizei actualLength = 0;
            glGetActiveUniformName(program, activeUniformIndices[i], static_cast<GLsizei>(uniformNameBuffer.size()), &actualLength,
                uniformNameBuffer.data());

            bufferFields.emplace_back(
                std::string{uniformNameBuffer.begin(), uniformNameBuffer.begin() + actualLength},
                 activeUniformOffsets[i],
                ArrayAttributes{static_cast<unsigned int>(activeUniformArraySizes[i]), static_cast<unsigned int>(activeUniformArrayStrides[i])},
                activeUniformMatrixStrides[i]
            );
        }

        return UniformBlockInfo {
            .Name = getUniformBlockName(program, blockBinding),
            .BlockIndex = static_cast<GLint>(blockIndex),
            .DataSize = static_cast<GLuint>(blockDataSize),
            .InitialBinding = static_cast<GLuint>(blockBinding),
            .Layout = {std::move(bufferFields)}
        };
    }
}; // namespace


std::unique_ptr<ProgramInfo> ProgramInfo::Request(GLuint program)
{
    GLint numActiveUniformBlocks, numActiveUniforms, uniformNameBufferLength;
    glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCKS, &numActiveUniformBlocks);
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &numActiveUniforms);
    glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &uniformNameBufferLength);

    const auto freeUniformsIndices = [&]{
        std::vector<GLint> result;
        for (GLint uniformIndex = 0; uniformIndex < numActiveUniforms; ++uniformIndex)
        {
            const GLuint uniformIndexU = static_cast<GLuint>(uniformIndex);
            GLint blockIndex;
            glGetActiveUniformsiv(program, 1, &uniformIndexU, GL_UNIFORM_BLOCK_INDEX, &blockIndex);

            if(blockIndex == GL_INVALID_INDEX)
                result.push_back(uniformIndex);
        }

        return result;
    }();

    std::string uniformNameBuffer(uniformNameBufferLength, '\0');
    
    auto programInfo = std::unique_ptr<ProgramInfo> { new ProgramInfo() };
    for (size_t i = 0; i < freeUniformsIndices.size(); ++i)
    {
        GLsizei actualLength, arraySize;
        GLenum type;
        glGetActiveUniform(program, freeUniformsIndices[i], static_cast<GLsizei>(uniformNameBuffer.size()), &actualLength, &arraySize, &type, uniformNameBuffer.data());

        const auto location = glGetUniformLocation(program, uniformNameBuffer.data());
        programInfo->uniforms.try_emplace(std::string{uniformNameBuffer.begin(), uniformNameBuffer.begin() + actualLength}, UniformInfo {location, arraySize, static_cast<UniformInfo::UniformType>(type)});
    }

    for (GLint blockIndex = 0; blockIndex < numActiveUniformBlocks; ++blockIndex)
    {
        auto [blockIt, _] = programInfo->uniformBlocks.emplace(blockIndex, getUniformBlockInfo(program, blockIndex, uniformNameBuffer));
        programInfo->uniformBlocksByName.try_emplace(blockIt->second.Name, &blockIt->second);
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
