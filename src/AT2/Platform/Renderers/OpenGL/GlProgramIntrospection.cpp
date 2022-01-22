#include "GlProgramIntrospection.h"

using namespace std::literals;

namespace
{
    template <size_t N>
    std::array<GLint, N> getProgramResource(GLuint program, GLenum interface, GLuint index,
                                            const std::array<GLenum, N>& request)
    {
        auto result = std::array<GLint, N> {};
        glGetProgramResourceiv(program, interface, index, static_cast<GLsizei>(request.size()), request.data(),
                               static_cast<GLsizei>(result.size()), nullptr, result.data());

        return result;
    }
}; // namespace

using namespace AT2;
using namespace OpenGL::Introspection;

std::unique_ptr<ProgramInfo> ProgramInfo::Request(GLuint program)
{
    GLint numUniforms = 0;
    glGetProgramInterfaceiv(program, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms);

    constexpr std::array<GLenum, 8> uniformAttributes {
        GL_NAME_LENGTH, GL_LOCATION, GL_TYPE, GL_ARRAY_SIZE, GL_BLOCK_INDEX, GL_OFFSET, GL_ARRAY_STRIDE, GL_MATRIX_STRIDE
    };

    auto programInfo = std::unique_ptr<ProgramInfo> { new ProgramInfo() };

    std::unordered_map<int, std::vector<Field>> layoutsData;

    for (GLuint uniformIndex = 0; uniformIndex < static_cast<GLuint>(numUniforms); ++uniformIndex)
    {
        const auto [nameLength, location, type, arraySize, blockIndex, offset, arrayStride, matrixStride] =
            getProgramResource(program, GL_UNIFORM, uniformIndex, uniformAttributes);

        assert(arraySize > 0);

        //ask a uniform's name
        std::string uniformName(static_cast<size_t>(nameLength)-1, '\0');  //because nameLength includes \0
        glGetProgramResourceName(program, GL_UNIFORM, uniformIndex, nameLength, nullptr, uniformName.data());

        if (uniformName.ends_with("[0]"sv))
            uniformName.resize(uniformName.length() - 3);

        if (blockIndex == static_cast<GLint>(GL_INVALID_INDEX)) //it's free uniforms
        {
            programInfo->uniforms.try_emplace(std::move(uniformName), UniformInfo {location, arraySize, static_cast<UniformInfo::UniformType>(type)});
        }
        else //it's uniform from uniform block
        {
            assert(arraySize >= 0 && arrayStride >= 0);
            layoutsData[blockIndex].emplace_back(
                std::move(uniformName), offset,
                ArrayAttributes {static_cast<unsigned int>(arraySize), static_cast<unsigned int>(arrayStride)}, matrixStride);
        }
    }

    for (auto& [blockIndex, blockDescriptor] : layoutsData)
    {
        const auto [blockNameLength, dataSize, binding] =
            getProgramResource(program, GL_UNIFORM_BLOCK, static_cast<GLuint>(blockIndex),
                               std::array<GLenum, 3> {GL_NAME_LENGTH, GL_BUFFER_DATA_SIZE, GL_BUFFER_BINDING});

        assert(blockNameLength > 0);
        std::string blockName(static_cast<size_t>(blockNameLength) - 1, '\0'); //because nameLength includes \0
        glGetProgramResourceName(program, GL_UNIFORM_BLOCK, static_cast<GLuint>(blockIndex), blockNameLength, nullptr, blockName.data());


        //add it to lookup
        auto [blockIt, _] = programInfo->uniformBlocks.emplace(blockIndex, UniformBlockInfo {
        	std::move(blockName),
            blockIndex,
        	static_cast<GLuint>(dataSize),
        	static_cast<GLuint>(binding),
        	BufferLayout {std::move(blockDescriptor)}
        });
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
