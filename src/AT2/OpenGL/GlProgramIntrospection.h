#ifndef AT2_GL_PROGRAM_INTROSPECTION_H
#define AT2_GL_PROGRAM_INTROSPECTION_H

#include "AT2lowlevel.h"

namespace AT2::OpenGL::Introspection
{

    struct UniformInfo
    {
        GLint Location = 0;
        GLint Type = 0;
    };

    using UniformMap = std::unordered_map<std::string, UniformInfo>;


    struct BufferedUniformInfo : UniformInfo
    {
        GLint Offset = 0;
        GLint ArrayStride = 0;
        GLint MatrixStride = 0;
    };

    using BufferedUniformMap = std::unordered_map<std::string, BufferedUniformInfo>;


    struct UniformBlockInfo
    {
        std::string Name;
        GLint BlockIndex = 0;
        GLint DataSize = 0;
        GLint InitialBinding = 0;

        BufferedUniformMap Uniforms;
    };

    using UniformBlockMap = std::unordered_map<GLint, UniformBlockInfo>;


    class ProgramInfo
    {
    public:
        static std::unique_ptr<ProgramInfo> Request(GLuint program);

        const UniformMap& getUniforms() const noexcept { return uniforms; }
        const UniformBlockInfo* getUniformBlock(GLint blockIndex) const;
        const UniformBlockInfo* getUniformBlock(std::string_view blockName) const;

    private:
        ProgramInfo() = default;

        UniformMap uniforms;
        UniformBlockMap uniformBlocks;
        std::unordered_map<std::string_view, UniformBlockInfo*> uniformBlocksByName; //just another view of uniformBlocks
    };

} // namespace AT2::OpenGL::Introspection

#endif