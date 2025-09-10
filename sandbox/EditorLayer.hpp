#pragma once

#include <Engine.hpp>

#include "editor/NodeEditPanel.hpp"
#include "editor/NodeListPanel.hpp"
#include "editor/ResourceAccess.hpp"
#include "editor/imfilebrowser.h"

namespace Galaxy {
enum class EditorMode {
    Edit,
    Run
};

class EditorLayer : public Layer {
public:
    EditorLayer(const char* projectPath = "");
    ~EditorLayer() override;

    void onAttach() override;
    void onDetach() override;
    void onUpdate() override;
    void onImGuiRender() override;
    void onEvent(Event& evt) override;

private:
    // Used to ask Project if the scene is still valid
    uuid m_selectedSceneId;
    Scene* m_selectedScene;

    NodeListPanel m_nodeList;
    NodeEditPanel m_editNode;
    ResourceAccess m_resourceAccess;
    ImGui::FileBrowser m_fileDialog;

    vec2 m_viewportSize;
    FrameBuffer* m_viewportFrame;

    EditorMode m_mode;
    bool m_showAllNodes;

    // Ref to nodes in Root
    std::shared_ptr<Node> m_rootEditorNode;
    std::shared_ptr<Camera> m_editorCamera;
    std::shared_ptr<Node> m_rootSceneNode;

    float m_cameraSpeed   = 0.15f;
    bool m_rightClickDown = false;

    void updateCamera();
    void displayMenuBar(bool validScene);
    void displayViewport(bool sceneSelected);
};
}