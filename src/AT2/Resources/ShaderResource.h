#pragma once

#include "ResourceManager.h"
#include "DataSource.h"

namespace AT2::Resources
{
    class ShaderResource final : public IResource
    {
    public:
        ShaderResource(std::vector<std::unique_ptr<IDataSource>> _sources, std::string name = {}) noexcept :
            m_sources(_sources.size()),
            m_name(std::move(name))
        {
            std::move(_sources.begin(), _sources.end(), m_sources.begin());
        }

        [[nodiscard]] std::string_view GetName() const noexcept override { return m_name; }
        std::shared_ptr<void> Load(IResourceFactory& resourceFactory) override;

    private:
        std::vector<std::unique_ptr<IDataSource>> m_sources;
        std::string m_name;
    };

    class ShaderResourceBuilder final
    {
    public:
        NON_COPYABLE_OR_MOVABLE(ShaderResourceBuilder)

        ShaderResourceBuilder(std::string name = {}) : m_name(std::move(name)) {}

        ShaderResourceBuilder& addSource(std::unique_ptr<IDataSource> dataSource)
        {
            m_sources.push_back(std::move(dataSource));
            return *this;
        }

        [[nodiscard]] std::unique_ptr<ShaderResource> build()
        {
            return std::make_unique<ShaderResource>(std::move(m_sources), std::move(m_name));
        }

    private:
        std::vector<std::unique_ptr<IDataSource>> m_sources;
        std::string m_name;
    };
};