#pragma once

#include <map>
#include "AT2lowlevel.h"

namespace AT2::Metal
{
    class Renderer;

    class ShaderProgram : public IShaderProgram, public std::enable_shared_from_this<ShaderProgram>
    {
    public:
        NON_COPYABLE_OR_MOVABLE(ShaderProgram)

        ShaderProgram(Renderer& renderer);
        ~ShaderProgram() override;

    public:
        unsigned int GetId() const noexcept override { return 0; }
        bool IsActive() const noexcept override;
        std::unique_ptr<IUniformContainer> CreateAssociatedUniformStorage(std::string_view blockName) override;

        void AttachShader(std::string_view data, ShaderType type) override;

        //Warning: Shader reloading/relinking will invalidate that state
        void SetUBO(std::string_view blockName, unsigned int index) override;
        void SetUniform(std::string_view name, Uniform value) override;
        void SetUniformArray(std::string_view name, UniformArray value) override;

        virtual const str& GetName() { return m_name; }
        virtual void SetName(const str& name) { m_name = name; }

        MtlPtr<MTL::Library> GetLibrary() { return m_library; }
        
    protected:
        bool TryCompile();
        void CleanUp();

    private:
        Renderer& m_renderer;
        str m_name;
        
        
        MtlPtr<MTL::Library> m_library;
    };
} // namespace AT2::Metal
