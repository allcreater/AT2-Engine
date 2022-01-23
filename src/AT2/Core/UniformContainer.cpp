#include "UniformContainer.h"

#include <utility>

using namespace AT2;
using namespace glm;

namespace
{
    // Technically it should be just insert_or_assign(name, std::move(value)); but seems that C++20's authors just forgot about heterohenous lookup :(
    const auto insert_or_assign = []<typename T>(auto& map, const auto& name, T&& value) {
        const auto& [it, isInserted] = map.emplace(name, value); //try_replace is not works too, so that we could not move =(
        if (!isInserted)
            it->second = std::forward<T>(value);
    };
}

void UniformContainer::Commit(const std::function<void(IUniformsWriter&)>& commitFunc) 
{
    class UniformContainerWriter : public IUniformContainer::IUniformsWriter
    {
    public:
        UniformContainerWriter(UniformContainer& uniformContainer) : m_container {uniformContainer} {}

        void Write(std::string_view name, Uniform value) override
        {
            insert_or_assign(m_container.m_uniformsMap, name, std::move(value));
        }

        void Write(std::string_view name, UniformArray value) override 
        {
        	throw AT2NotImplementedException("UniformContainerWriter::Write(UniformArray)");
        }

        void Write(std::string_view name, std::shared_ptr<ITexture> value) override
        {
            insert_or_assign(m_container.m_texturesMap, name, std::move(value));
        }

    private:
        UniformContainer& m_container;
    } writer {*this};

    commitFunc(writer);
}

void UniformContainer::Bind(IStateManager& stateManager) const
{
    const auto& program = stateManager.GetActiveShader();
    if (!program)
        throw AT2ShaderException("GlUniformContainer: try to bind uniform container when parent program is missing");

    for (const auto& [name, value] : m_uniformsMap)
        program->SetUniform(name, value);

    TextureSet set; //TODO: just a crutch, make it correct!!!
    for (const auto& [name, value] : m_texturesMap)
        set.insert(value);
    stateManager.BindTextures(set);

    for (const auto& [name, value] : m_texturesMap)
        program->SetUniform(name, static_cast<int>(*stateManager.GetActiveTextureIndex(value)));
}
