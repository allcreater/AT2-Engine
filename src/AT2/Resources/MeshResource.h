#pragma once

#include "DataSource.h"
#include "ResourceManager.h"

namespace AT2::Resources
{
    class MeshResource final : public IResource
    {
        MeshResource(std::unique_ptr<IDataSource> source, std::string name = {}) noexcept :
            m_source(std::move(source)), m_name(std::move(name))
        {
        }

    public:
        [[nodiscard]] std::string_view GetName() const noexcept override { return m_name; }
        std::shared_ptr<void> Load(const std::shared_ptr<IResourceFactory>& resourceFactory) override;

        static std::unique_ptr<MeshResource> Make(std::unique_ptr<IDataSource> source, std::string name = {})
        {
            return std::unique_ptr<MeshResource> {new MeshResource(std::move(source), std::move(name))};
        }

    private:
        std::unique_ptr<IDataSource> m_source;
        std::string m_name;
    };

} // namespace AT2::Resources