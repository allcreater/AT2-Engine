#include <Camera.h>
#include <Platform/Application.h>
#include <Resources/TextureLoader.h>
#include <glm/gtx/euler_angles.hpp>

#include "../mesh_renderer.h"

class CubeExample final : public AT2::WindowContextBase
{
public:
    CubeExample() = default;

private:
    static AT2::Mesh BuildCube(AT2::IResourceFactory& resourceFactory)
    {
        // Arrays from the example: https://github.com/glcoder/gl33lessons/blob/wiki/Lesson03.md

        constexpr float s = 1.0f;
        constexpr float cubePositions[][3] = {
			{-s, s, s}, { s, s, s}, { s,-s, s}, {-s,-s, s}, // front
			{ s, s,-s}, {-s, s,-s}, {-s,-s,-s}, { s,-s,-s}, // back
			{-s, s,-s}, { s, s,-s}, { s, s, s}, {-s, s, s}, // top
			{ s,-s,-s}, {-s,-s,-s}, {-s,-s, s}, { s,-s, s}, // bottom
			{-s, s,-s}, {-s, s, s}, {-s,-s, s}, {-s,-s,-s}, // left
			{ s, s, s}, { s, s,-s}, { s,-s,-s}, { s,-s, s}  // right
		};
        constexpr float cubeTexcoords[][2] = {
            {0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f}, // front
            {0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f}, // back
            {0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f}, // top
            {0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f}, // bottom
            {0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f}, // left
            {0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f}  // right
        };

		constexpr uint32_t cubeIndices[] = {
			 0, 3, 1,  1, 3, 2, // front
			 4, 7, 5,  5, 7, 6, // back
			 8,11, 9,  9,11,10, // top
			12,15,13, 13,15,14, // bottom
			16,19,17, 17,19,18, // left
			20,23,21, 21,23,22  // right
		};

        auto vao = MakeVertexArray(resourceFactory, 
			std::make_pair(1u, std::cref(cubePositions)),
			std::make_pair(2u, std::cref(cubeTexcoords))
        );

        vao->SetIndexBuffer(resourceFactory.MakeBufferFrom(AT2::VertexBufferFlags::IndexBuffer, cubeIndices), AT2::BufferDataType::UInt);

        AT2::SubMesh subMesh;
        subMesh.Primitives.push_back({AT2::Primitives::Triangles {}, 0, std::size(cubeIndices)});

        AT2::Mesh mesh {};
        mesh.VertexArray = std::move(vao);
        mesh.SubMeshes.push_back(std::move(subMesh));

        return mesh;
    }


    void OnInitialized( AT2::IVisualizationSystem& visualizationSystem ) override
    {
        getWindow().setVSyncInterval(0).setCursorMode(CursorMode::Hidden);

        visualizationSystem.GetDefaultFramebuffer().SetClearColor(glm::vec4{0.0, 0.0, 0.5, 0.0});
        visualizationSystem.GetDefaultFramebuffer().SetClearDepth(1.0f);

        m_camera.setView(lookAt(glm::vec3 {5, 1, 0}, glm::vec3 {0, 0, 0}, glm::vec3 {0, 1, 0}));

        m_cubeMesh        = BuildCube(visualizationSystem.GetResourceFactory());
        m_cubeMesh.Shader = visualizationSystem.GetResourceFactory().CreateShaderProgramFromFiles(
            {"resources/simple_mesh.vs.glsl", "resources/simple_mesh.fs.glsl"});

        auto texture = AT2::Resources::TextureLoader::LoadTexture(visualizationSystem, "../../resources/Ground037_2K-JPG/Ground037_2K_Color.jpg");
        m_cubeMesh.GetOrCreateDefaultMaterial().SetUniform("u_texAlbedo", std::move(texture));
    }

    void OnRender( AT2::Seconds dt, AT2::IVisualizationSystem& visualizationSystem ) override
    {
        visualizationSystem.GetDefaultFramebuffer().Render([&](AT2::IRenderer& renderer){
            auto& stateManager = renderer.GetStateManager();
    	    stateManager.ApplyState(AT2::DepthState { AT2::CompareFunction::Less, true, true});
            stateManager.ApplyState(AT2::FaceCullMode {false, true});

    	    AT2::Utils::MeshRenderer::DrawMesh(renderer, m_cubeMesh, m_cubeMesh.Shader );
        });
    }

    void OnKeyDown(int key) override
    {
    }

    void OnResize(glm::ivec2 newSize) override
    {
        if (newSize.x <= 0 || newSize.y <= 0)
            return;

        m_camera.setProjection(
            glm::perspectiveFov(glm::radians(60.0f), static_cast<float>(newSize.x), static_cast<float>(newSize.y), 0.1f, 200.0f));
    }

    void OnMouseUp(int key) override
    {
    }

    void OnMouseMove(const AT2::MousePos& pos) override
    {
        const auto angularSpeed = glm::vec3 {pos.getDeltaPos(), 0.0f} * 0.004f;
        m_cubeRotation = glm::normalize(m_cubeRotation * glm::quat {angularSpeed});
    }

    void OnUpdate(AT2::Seconds dt) override
    {
        if (getWindow().isKeyDown(AT2::Keys::Key_Escape))
            getWindow().setCloseFlag(true);

        using namespace std::chrono_literals;
        const auto angularSpeed = glm::vec3 {1.0f, 1.3f, 1.2f} * static_cast<float>(dt/1s);
        m_cubeRotation = glm::normalize(m_cubeRotation * glm::quat {angularSpeed});


    	m_cubeMesh.GetOrCreateDefaultMaterial().Commit([&](AT2::IUniformsWriter& writer) {
            writer.Write("u_matModelView", m_camera.getView() * glm::mat4 {m_cubeRotation});
            writer.Write("u_matProjection", m_camera.getProjection());
        });
    }

private:
    AT2::Mesh m_cubeMesh;
    AT2::Camera m_camera;
    glm::quat m_cubeRotation;
};
