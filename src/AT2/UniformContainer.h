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
	void SetUniform(const str& name, const Uniform& value) override;
	void SetUniform(const str& name, const std::shared_ptr<ITexture> &value) override;

	void Bind(IStateManager &stateManager) const override;

private:
	std::unordered_map<str, Uniform> m_uniformsMap;
	std::unordered_map<str, std::shared_ptr<const ITexture>> m_texturesMap;
};

}

#endif