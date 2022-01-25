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
        void Commit(const std::function<void(IUniformsWriter&)>& writer) override;

        void Bind(IStateManager& stateManager) const override;

    private:
        using UniformVariant = std::variant<Uniform, UniformArray, std::shared_ptr<IBuffer>, std::shared_ptr<ITexture>>;
        Utils::UnorderedStringMap<UniformVariant> m_uniformsMap;
    };

} // namespace AT2

#endif
