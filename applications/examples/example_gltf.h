#pragma once

#include <Camera.h>
#include <Platform/Application.h>
#include <Resources/TextureLoader.h>
#include <Resources/GltfSceneLoader.h>
//#include <Scene/Scene.h>

#include "../mesh_renderer.h"
#include "../sandbox/SceneRenderer.h"

class GltfSceneExample final : public AT2::WindowContextBase
{
public:
    GltfSceneExample() = default;

private:
    void OnInitialized( AT2::IVisualizationSystem& visualizationSystem ) override
    {
        getWindow().setVSyncInterval(0).setCursorMode(CursorMode::Hidden);

        visualizationSystem.GetDefaultFramebuffer().SetClearColor(glm::vec4{0.0, 0.0, 0.5, 0.0});
        visualizationSystem.GetDefaultFramebuffer().SetClearDepth(1.0f);

        m_camera.setView(lookAt(glm::vec3 {5, 1, 0}, glm::vec3 {0, 0, 0}, glm::vec3 {0, 1, 0}));

        {
           auto scene = AT2::Resources::GltfMeshLoader::LoadScene(
            //    visualizationSystem, R"(/Volumes/DATA/git/glTF-Sample-Models/2.0/BoxTextured/glTF/BoxTextured.gltf)");
            //    visualizationSystem, R"(/Volumes/DATA/git/glTF-Sample-Models/2.0/Avocado/glTF/Avocado.gltf)");
            //   visualizationSystem, R"(/Volumes/DATA/git/glTF-Sample-Models/2.0/Lantern/glTF/Lantern.gltf)");
                visualizationSystem, R"(/Volumes/DATA/git/glTF-Sample-Models/2.0/Sponza/glTF/Sponza.gltf)");
           //scene->GetTransform().setScale({50.0, 50.0, 50.0}).setPosition({0, 0, 0});
            //scene->GetTransform().setScale({0.1, 0.1, 0.1}).setPosition({0, 0, 0});
            m_scene.GetRoot().AddChild(scene);
        }
        

        // {
        //     auto node = std::make_shared<AT2::Scene::Node>();
        //     auto mesh = std::shared_ptr{AT2::Utils::MakeCubeMesh(visualizationSystem.GetResourceFactory())};
        //     node->getOrCreateComponent<AT2::Scene::MeshComponent>(mesh, std::vector<unsigned int>{0});
            
        //     m_scene.GetRoot().AddChild(std::move(node));
        // }

        meshShader = visualizationSystem.GetResourceFactory().CreateShaderProgramFromFiles(
            {"../../resources/shaders/mesh.vs.glsl", "../../resources/shaders/mesh.fs.glsl"});

        constexpr glm::ivec4 emptydata[1] = {{0, 0, 0, 0}};
        auto appendVertexArrayWithEmptyAttributes = [emptyBuffer = visualizationSystem.GetResourceFactory().MakeBufferFrom(AT2::VertexBufferFlags::Immutable, emptydata)](AT2::IVertexArray& vao) {
            vao.SetAttributeBinding(4, emptyBuffer, AT2::BufferBindingParams{AT2::BufferDataType::UInt, 4, sizeof(emptydata), 0, false, 1});
            vao.SetAttributeBinding(5, emptyBuffer, AT2::BufferBindingParams{AT2::BufferDataType::Float, 4, sizeof(emptydata), 0, false, 1});
        };

        AT2::Scene::FuncNodeVisitor pipelineSetterVisitor {[&, this](AT2::Scene::Node& node) {
            for (auto* meshComponent : node.getComponents<AT2::Scene::MeshComponent>())
            {
                //TODO: don't create duplicates
                    meshComponent->getMesh()->PipelineState = visualizationSystem.GetResourceFactory().CreatePipelineState(
                        AT2::PipelineStateDescriptor()
                        .SetShader(meshShader)
                        .SetVertexArray(meshComponent->getMesh()->VertexArray)
                        .SetDepthState({ AT2::CompareFunction::Less, true, true}));

                appendVertexArrayWithEmptyAttributes(*meshComponent->getMesh()->VertexArray);

                if (!cameraUniformBuffer)
                    cameraUniformBuffer = meshShader->CreateAssociatedUniformStorage("CameraBlock");
                    
                meshComponent->getMesh()->GetOrCreateDefaultMaterial().SetUniform("CameraBlock", cameraUniformBuffer->GetBuffer());
            }

            return true;
        }};

        m_scene.GetRoot().Accept(pipelineSetterVisitor);        
    }

    void OnUpdate(AT2::Seconds dt) override
    {
        if (getWindow().isKeyDown(AT2::Keys::Key_Escape))
            getWindow().setCloseFlag(true);

        AT2::Scene::SceneRenderer::SetupCamera(*cameraUniformBuffer, m_camera, nullptr);

    }

    void OnRender( AT2::Seconds dt, AT2::IVisualizationSystem& visualizationSystem ) override
    {
        visualizationSystem.GetDefaultFramebuffer().Render([&](AT2::IRenderer& renderer){
            renderer.GetStateManager().ApplyState(AT2::FaceCullMode {false, true});

            AT2::Scene::RenderVisitor rv {renderer, m_camera};
            m_scene.GetRoot().Accept(rv);
        });
    }

    void OnKeyDown(int key) override
    {
    }

    void OnResize(glm::ivec2 newSize) override
    {
        m_camera.setProjection(
            glm::perspectiveFov(glm::radians(60.0f), static_cast<float>(newSize.x), static_cast<float>(newSize.y), 0.1f, 2000.0f));
    }

    void OnMouseUp(int key) override
    {
    }

    void OnMouseMove(const AT2::MousePos& pos) override 
    {
        const auto relativePos = pos.getPos() / static_cast<glm::dvec2>(getWindow().getSize());

        m_camera.setRotation(glm::angleAxis(glm::mix(-glm::pi<float>(), glm::pi<float>(), relativePos.x),
                                            glm::vec3 {0.0, -1.0, 0.0}) *
                                glm::angleAxis(glm::mix(-glm::pi<float>() / 2, glm::pi<float>() / 2, relativePos.y),
                                            glm::vec3 {1.0, 0.0, 0.0}));
    }

private:
    AT2::Camera m_camera;
    AT2::Scene::Scene m_scene;
    std::shared_ptr<AT2::IShaderProgram> meshShader;
    std::unique_ptr<AT2::StructuredBuffer> cameraUniformBuffer;
};
