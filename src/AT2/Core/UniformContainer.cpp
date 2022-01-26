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
    class UniformContainerWriter : public IUniformsWriter
    {
    public:
        UniformContainerWriter(UniformContainer& uniformContainer) : m_container {uniformContainer} {}

        void Write(std::string_view name, Uniform value) override { insert_or_assign(m_container.m_uniformsMap, name, std::move(value)); }

        void Write(std::string_view name, UniformArray value) override
        {
            insert_or_assign(m_container.m_uniformsMap, name, std::move(value));
        }

        void Write(std::string_view name, std::shared_ptr<ITexture> value) override
        {
            insert_or_assign(m_container.m_uniformsMap, name, std::move(value));
        }

        void Write(std::string_view name, std::shared_ptr<IBuffer> value) override
        {
            insert_or_assign(m_container.m_uniformsMap, name, std::move(value));
        }

    private:
        UniformContainer& m_container;
    } writer {*this};

    commitFunc(writer);
}

void UniformContainer::Bind(IStateManager& stateManager) const
{
    stateManager.Commit([this](IUniformsWriter& writer) 
    {
        for (const auto& [name, valueVariant] : m_uniformsMap)
            std::visit([name = name, &writer](const auto& value) { writer.Write(name, value); }, valueVariant);
    });

}
