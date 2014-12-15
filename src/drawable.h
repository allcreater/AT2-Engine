#ifndef AT2_DRAWABLE_CLASS
#define AT2_DRAWABLE_CLASS

#include "AT2.h"

namespace AT2
{
class IDrawable
{
public:
	virtual void Draw (IRenderer& renderer) = 0;

private:

};

class MeshDrawable : public IDrawable //mesh или не mesh, но определенно что-то похожее
{
public:
	void Draw(IRenderer& renderer) override
	{
		auto stateManager = renderer.GetStateManager();

		stateManager->BindShader(Shader);
		stateManager->BindVertexArray(VertexArray);
		stateManager->BindTextures(Textures);
		
		UniformBuffer->Bind();

		for(auto primitive: Primitives)
			primitive->Draw();
	}

	~MeshDrawable() {}

	std::shared_ptr<IShaderProgram> Shader;
	std::shared_ptr<IVertexArray> VertexArray;
	std::shared_ptr<IUniformContainer> UniformBuffer;
	TextureSet Textures;
	PrimitiveList Primitives;

private:
	
};
}
#endif