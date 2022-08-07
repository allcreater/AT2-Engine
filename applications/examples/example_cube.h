#pragma once

#include <Camera.h>
#include <Platform/Application.h>
#include <Resources/TextureLoader.h>
#include <glm/gtx/euler_angles.hpp>

#include "../mesh_renderer.h"
#include "../procedural_meshes.h"

class CubeExample final : public AT2::WindowContextBase
{
public:
    CubeExample() = default;

private:
    void OnInitialized( AT2::IVisualizationSystem& visualizationSystem ) override
    {
        getWindow().setVSyncInterval(0).setCursorMode(CursorMode::Hidden);

        visualizationSystem.GetDefaultFramebuffer().SetClearColor(glm::vec4{0.0, 0.0, 0.5, 0.0});
        visualizationSystem.GetDefaultFramebuffer().SetClearDepth(1.0f);

        m_camera.setView(lookAt(glm::vec3 {5, 1, 0}, glm::vec3 {0, 0, 0}, glm::vec3 {0, 1, 0}));

        m_cubeMesh = AT2::Utils::MakeCubeMesh(visualizationSystem.GetResourceFactory());
        
        auto shader = visualizationSystem.GetResourceFactory().CreateShaderProgramFromFiles(
            {"resources/simple_mesh.vs.glsl", "resources/simple_mesh.fs.glsl"});

        m_cubeMesh->PipelineState = visualizationSystem.GetResourceFactory().CreatePipelineState(
            AT2::PipelineStateDescriptor().SetShader(shader)
                .SetVertexArray(m_cubeMesh->VertexArray)
                .SetDepthState({ AT2::CompareFunction::Less, true, true})
        );

        auto texture = AT2::Resources::TextureLoader::LoadTexture(visualizationSystem, "../../resources/Ground037_2K-JPG/Ground037_2K_Color.jpg");
        m_cubeMesh->GetOrCreateDefaultMaterial().SetUniform("u_texAlbedo", std::move(texture));
    }

    void OnRender( AT2::Seconds dt, AT2::IVisualizationSystem& visualizationSystem ) override
    {
        visualizationSystem.GetDefaultFramebuffer().Render([&](AT2::IRenderer& renderer){
            auto& stateManager = renderer.GetStateManager();
            stateManager.ApplyState(AT2::FaceCullMode {false, true});

    	    AT2::Utils::MeshRenderer::DrawMesh(renderer, *m_cubeMesh, m_cubeMesh->PipelineState );
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

    	m_cubeMesh->GetOrCreateDefaultMaterial().Commit([&](AT2::IUniformsWriter& writer) {
            writer.Write("u_matModelView", m_camera.getView() * glm::mat4 {m_cubeRotation});
            writer.Write("u_matProjection", m_camera.getProjection());
        });
    }

private:
    std::unique_ptr<AT2::Mesh> m_cubeMesh;
    AT2::Camera m_camera;
    glm::quat m_cubeRotation;
};
