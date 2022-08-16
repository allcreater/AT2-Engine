#ifndef AT2_GL_PROGRAM_INTROSPECTION_H
#define AT2_GL_PROGRAM_INTROSPECTION_H

#include "AT2lowlevel.h"

#include <DataLayout/BufferLayout.h>

namespace AT2::OpenGL41::Introspection
{

    struct UniformInfo
    {
        enum class UniformType : GLint { None = -1 };

        GLint Location = 0;
        GLint ArraySize = 0;
        UniformType Type = UniformType::None;
    };

    using UniformMap = Utils::UnorderedStringMap<UniformInfo>;

    struct UniformBlockInfo
    {
        std::string Name;
        GLint BlockIndex = 0;
        GLuint DataSize = 0;
        GLuint InitialBinding = 0;

        BufferLayout Layout;
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

} // namespace AT2::OpenGL41::Introspection

#endif
