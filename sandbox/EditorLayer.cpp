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
    m_viewportFrame = new FrameBuffer(320, 180, FramebufferTextureFormat::RGBA8);
}

void EditorLayer::onDetach()
{
    m_viewportFrame->destroy();
}

void EditorLayer::onUpdate()
{
    m_viewportFrame->bind();
    if (m_selectedScene.getNodePtr()) {
        auto& renderer = Renderer::getInstance();
        mat4 cameraTransform;
        if (m_mode == EditorMode::Run) {
            cameraTransform = CameraManager::getInstance().getCurrentCamTransform();
        } else {
            cameraTransform = m_editorCamera->getTransform()->getGlobalModelMatrix();
        }
        renderer.beginSceneRender(cameraTransform);

        m_selectedScene.getNodePtr()->draw();

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
            if (m_selectedScene.isValid() && ImGui::MenuItem("Save")) {
                m_selectedScene.save();
            }
            if (ImGui::MenuItem("Load")) {
                if (!m_selectedScene.load("TEST.glx")) {
                    GLX_CORE_ERROR("Failed to load '{0}'", "TEST.glx");
                } else {
                    m_rootSceneNode->clearChilds();
                    m_rootSceneNode->addChild(m_selectedScene.getNodePtr());
                }
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();

        ImGui::End();
    }

    ImGui::Begin("Application state");
    ImGui::Text((std::string("FPS: ") + std::to_string(1.0 / Application::getInstance().getDelta())).c_str());
    ImGui::End();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2 { 0, 0 });
    ImGui::Begin("Viewport");
    if (m_selectedScene.getNodePtr().get()) {
        if (ImGui::Button("Run")) {
            m_mode = EditorMode::Run;
            m_rootSceneNode->activate();
            m_rootEditorNode->disable();
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop")) {
            m_mode = EditorMode::Edit;
            m_rootSceneNode->disable();
            m_rootEditorNode->activate();
        }
        if (!ImGui::IsWindowDocked()) {
            ImGui::PopStyleVar();
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2 { 100, 1000 });
        }
        ImVec2 pannelSize = ImGui::GetContentRegionAvail();
        if (m_viewportSize != *(vec2*)&pannelSize) {
            m_viewportSize = { pannelSize.x, pannelSize.y };
            m_viewportFrame->resize(m_viewportSize.x, m_viewportSize.y);
        }
        ImGui::PopStyleVar();
        auto textureID = m_viewportFrame->getColorTextureID();
        ImGui::Image(reinterpret_cast<void*>(textureID), pannelSize, ImVec2 { 0, 0 }, ImVec2 { 1, 1 });
    } else {
        ImGui::PopStyleVar();
    }

    ImGui::End();

    ImGui::Begin("Scene");

    m_nodeList.listNodes(*m_rootSceneNode.get());
    if (Node::nodeExists(m_nodeList.selectedNodeId)) {
        m_editNode.selectNode(*Node::getNode(m_nodeList.selectedNodeId).lock().get());
    }
    ImGui::End();
}

void EditorLayer::onEvent(Event& evt)
{
    if (evt.isInCategory(EventCategory::EventCategoryKeyboard)) {
        KeyEvent& keyEvt = (KeyEvent&)evt;
        if (!keyEvt.isPressed())
            return;

        if (keyEvt.getKeyCode() == 90) {
            m_editorCamera->translate(vec3(0, 0, 1));
        } else if (keyEvt.getKeyCode() == 83) {
            m_editorCamera->translate(vec3(0, 0, -1));
        } else if (keyEvt.getKeyCode() == 81) {
            m_editorCamera->translate(vec3(-1, 0, 0));
        } else if (keyEvt.getKeyCode() == 68) {
            m_editorCamera->translate(vec3(1, 0, 0));
        }
    }
}
}