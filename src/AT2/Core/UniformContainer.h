#ifndef AT2_UNIFORMCONTAINER_H
#define AT2_UNIFORMCONTAINER_H

#include "AT2.h"

namespace AT2
{

    class UniformContainer : public IUniformContainer
    {
    public:
        UniformContainer() = default;

    public:
        void SetUniform(std::string_view name, const Uniform& value) override;
        void SetUniform(std::string_view name, const std::shared_ptr<ITexture>& value) override;

        void Bind(IStateManager& stateManager) const override;

    private:
        Utils::UnorderedStringMap<Uniform> m_uniformsMap;
        Utils::UnorderedStringMap<std::shared_ptr<ITexture>> m_texturesMap;
    };

} // namespace AT2

#endif
