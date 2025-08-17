#include "EditorLayer.hpp"
#include "editor/NodeListPanel.hpp"
#include <Engine.hpp>

#include <imgui.h>

namespace Galaxy {
EditorLayer::~EditorLayer()
{
}

void EditorLayer::onAttach()
{
    m_rootEditorNode = std::make_shared<Node>("Editor");
    m_editorCamera   = std::make_shared<Camera>();
    m_editorCamera->setCurrent(false);
    m_rootEditorNode->addChild(m_editorCamera);
    m_rootEditorNode->activate();

    m_rootSceneNode = std::make_shared<Node>("RootScene");
    m_rootSceneNode->disable();

    std::shared_ptr<Node> rootNode = std::make_shared<Node>("Root");
    rootNode->addChild(m_rootEditorNode);
    rootNode->addChild(m_rootSceneNode);

    Application::getInstance().setRootNode(rootNode);

    auto& renderer  = Renderer::getInstance();
    m_viewportFrame = new FrameBuffer(320, 180, FramebufferTextureFormat::DEPTH24STENCIL8);

    InputManager::addAction(Action(GLX_KEY_ESCAPE, "editor_exit"));
    InputManager::addAction(Action(GLX_KEY_W, "editor_forward"));
    InputManager::addAction(Action(GLX_KEY_S, "editor_backward"));
    InputManager::addAction(Action(GLX_KEY_A, "editor_right"));
    InputManager::addAction(Action(GLX_KEY_D, "editor_left"));

    std::string projectPath(".gproj");
    if (!Project::load(projectPath)) {
        GLX_ERROR("Can't load project '{0}'", projectPath);
        Project::create(".gproj");
        Application::getInstance().terminate();
    }
}

void EditorLayer::onDetach()
{
    m_viewportFrame->destroy();
}

void EditorLayer::onUpdate()
{
    if (Project::isSceneValid(m_selectedSceneId)) {
        m_selectedScene = &Project::loadScene(m_selectedSceneId);
    } else {
        m_selectedScene = nullptr;
        return;
    }

    m_viewportFrame->bind();
    if (m_selectedScene->getNodePtr()) {
        auto& renderer = Renderer::getInstance();
        mat4 cameraTransform;
        if (m_mode == EditorMode::Run) {
            cameraTransform = CameraManager::getInstance().getCurrentCamTransform();
        } else {
            cameraTransform = m_editorCamera->getTransform()->getGlobalModelMatrix();
            updateCamera();
        }
        renderer.beginSceneRender(cameraTransform);

        m_selectedScene->getNodePtr()->draw();

        renderer.endSceneRender();
        renderer.renderFrame();
    }
    m_viewportFrame->unbind();
}

void EditorLayer::onImGuiRender()
{
    static bool dockSpaceOpen = true;
    if (dockSpaceOpen) {
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar;
        ImGuiViewport* viewport      = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus; // | ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Dock", &dockSpaceOpen, windowFlags);
        ImGui::PopStyleVar(1);

        ImGuiID dockspaceID = ImGui::GetID("DockSpace");
        ImGui::DockSpace(dockspaceID);

        ImGui::BeginMenuBar();
        if (ImGui::BeginMenu("File")) {
            if (m_selectedScene->isValid() && ImGui::MenuItem("Save")) {
                m_selectedScene->save();
            }
            ImGui::MenuItem("New scene");
            {
                static char sceneName[128];
                ImGui::InputText("Scene's path: ", sceneName, IM_ARRAYSIZE(sceneName));
                if (ImGui::Button("OK")) {
                    m_selectedScene   = &Project::createScene(std::string(sceneName));
                    m_selectedSceneId = m_selectedScene->getUuid();
                }
            }
            if (ImGui::BeginMenu("Load")) {
                int i = 0;
                for (auto& scene : Project::getPaths()) {
                    if (ImGui::MenuItem(std::string(scene.second + "##" + std::to_string(i++)).c_str())) {
                        m_selectedScene   = &Project::loadScene(scene.first);
                        m_selectedSceneId = m_selectedScene->getUuid();
                        m_rootSceneNode->clearChilds();
                        m_rootSceneNode->addChild(m_selectedScene->getNodePtr());
                    }
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();

        ImGui::End();
    }

    ImGui::Begin("App");
    ImGui::Text((std::string("FPS: ") + std::to_string(1.0 / Application::getInstance().getDelta())).c_str());
    ImGui::Checkbox("Show all nodes", &m_showAllNodes);
    ImGui::End();

    // ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2 { 0, 0 });
    // ImGui::Begin("Viewport");
    // if (m_selectedScene && m_selectedScene->getNodePtr().get()) {
    //     if (ImGui::Button("Run")) {
    //         m_mode = EditorMode::Run;
    //         m_rootSceneNode->activate();
    //         m_rootEditorNode->disable();
    //     }
    //     ImGui::SameLine();
    //     if (ImGui::Button("Stop")) {
    //         m_mode = EditorMode::Edit;
    //         m_rootSceneNode->disable();
    //         m_rootEditorNode->activate();
    //     }
    //     if (!ImGui::IsWindowDocked()) {
    //         ImGui::PopStyleVar();
    //         ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2 { 100, 1000 });
    //     }
    //     ImVec2 pannelSize = ImGui::GetContentRegionAvail();
    //     if (m_viewportSize != *(vec2*)&pannelSize) {
    //         m_viewportSize = { pannelSize.x, pannelSize.y };
    //         m_viewportFrame->resize(m_viewportSize.x, m_viewportSize.y);
    //     }
    //     ImGui::PopStyleVar();
    //     auto textureID = m_viewportFrame->getColorTextureID();
    //     ImGui::Image(reinterpret_cast<void*>(textureID), pannelSize, ImVec2 { 0, 0 }, ImVec2 { 1, 1 });
    // } else {
    //     ImGui::PopStyleVar();
    // }

    // ImGui::End();

    ImGui::Begin("Scene");

    if (m_showAllNodes)
        m_nodeList.listNodes(*Application::getInstance().getRootNodePtr());
    else
        m_nodeList.listNodes(*m_rootSceneNode.get());
    if (Node::nodeExists(m_nodeList.selectedNodeId)) {
        m_editNode.selectNode(*Node::getNode(m_nodeList.selectedNodeId).lock().get());
    }
    ImGui::End();
}

void EditorLayer::onEvent(Event& evt)
{
    if (evt.getEventType() == EventType::ActionInteract) {
        ActionEvent& actEvt = (ActionEvent&)evt;
        if (std::string(actEvt.getActionName()) == "editor_exit")
            Application::getInstance().terminate();
    }
}
void EditorLayer::updateCamera()
{
    const float speed = 0.2f;
    vec3 translation(0);
    if (InputManager::isActionPressed("editor_forward"))
        translation.z += speed;
    if (InputManager::isActionPressed("editor_backward"))
        translation.z -= speed;
    if (InputManager::isActionPressed("editor_right"))
        translation.x += speed;
    if (InputManager::isActionPressed("editor_left"))
        translation.x -= speed;

    m_editorCamera->translate(translation);
}
}