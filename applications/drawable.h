#ifndef AT2_DRAWABLE_CLASS
#define AT2_DRAWABLE_CLASS

#include <AT2/Drawable.h>

typedef AT2::VertexBufferType AT2vbt;

namespace AT2
{


class MeshDrawable : public IDrawable //mesh или не mesh, но определенно что-то похожее
{
public:
	void Draw(const std::shared_ptr<IRenderer>& renderer) override
	{
		auto& stateManager = renderer->GetStateManager();

		stateManager.BindShader(Shader);
		stateManager.BindVertexArray(VertexArray);
		stateManager.BindTextures(Textures);
		
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


public:

	static std::shared_ptr<AT2::MeshDrawable> MakeSphereDrawable(const std::shared_ptr<IRenderer>& renderer, int segX = 32, int segY = 16);
	static std::shared_ptr<AT2::MeshDrawable> MakeTerrainDrawable(const std::shared_ptr<IRenderer>& renderer, int segX, int segY);
	static std::shared_ptr<AT2::MeshDrawable> MakeFullscreenQuadDrawable(const std::shared_ptr<IRenderer>& renderer);
private:
	
};
}
#endif