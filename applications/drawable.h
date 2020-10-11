#ifndef AT2_DRAWABLE_CLASS
#define AT2_DRAWABLE_CLASS

#include <AT2/Drawable.h>

typedef AT2::VertexBufferType AT2vbt;

namespace AT2
{


    class MeshDrawable : public IDrawable
    {
    public:
        void Draw(const IRenderer& renderer) override
        {
            auto& stateManager = renderer.GetStateManager();

            stateManager.BindShader(Shader);
            stateManager.BindVertexArray(VertexArray);
            //stateManager.BindTextures(Textures);

            UniformBuffer->Bind(stateManager);

            for (auto* primitive : Primitives)
                primitive->Draw();
        }

        std::shared_ptr<IShaderProgram> Shader;
        std::shared_ptr<IVertexArray> VertexArray;
        std::shared_ptr<IUniformContainer> UniformBuffer;
        TextureSet Textures;
        std::vector<IDrawPrimitive*> Primitives;


    public:
        static std::shared_ptr<AT2::MeshDrawable> MakeSphereDrawable(const IRenderer& renderer, int segX = 32,
                                                                     int segY = 16);
        static std::shared_ptr<AT2::MeshDrawable> MakeFullscreenQuadDrawable(const IRenderer& renderer);
    };
} // namespace AT2
#endif