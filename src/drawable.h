#ifndef AT2_DRAWABLE_CLASS
#define AT2_DRAWABLE_CLASS

#include "AT2.h"

using namespace AT2;

class IDrawable
{
public:
	virtual void Draw (IRenderer& renderer) = 0;

private:

};


class StateSet //��������� �������
{
public:

};

class MeshDrawable : public IDrawable //mesh ��� �� mesh, �� ����������� ���-�� �������
{
public:
	void Draw(IRenderer& renderer)
	{
		auto stateManager = renderer.GetStateManager();

		stateManager->BindShader(Shader);
		stateManager->BindVertexArray(VertexArray);
		stateManager->BindTextures(Textures);
		
		for(auto primitive: Primitives)
			primitive->Draw();
	}

	~MeshDrawable() {}

	std::shared_ptr<IShaderProgram> Shader;
	std::shared_ptr<IVertexArray> VertexArray;
	TextureSet Textures;
	PrimitiveList Primitives;

private:
	
};

#endif