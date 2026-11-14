#pragma once

#include <Engine.hpp>

#include "src/CubemapCreation.hpp"
#include "src/NodeEditPanel.hpp"
#include "src/NodeListPanel.hpp"
#include "src/ResourceAccess.hpp"
#include "src/imfilebrowser.h"
#include "src/ImGui/ImGuiContext.hpp"
#include "ImGuizmo.h"

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
    void onImGuiRender();
    void onEvent(Event& evt) override;

    mat4 getEditorViewMatrix(){return m_editorCamera->getCamera()->getViews()[0].viewMatrix;}
    mat4 getEditorProjectionMatrix(){return m_editorCamera->getCamera()->getViews()[0].projectionMatrix;}

private:
    // Used to ask Project if the scene is still valid
    uuid m_selectedSceneId = 0;
    Scene* m_selectedScene  = nullptr;

    ImGuiContext m_imGuiContext;

    NodeListPanel m_nodeList;
    NodeEditPanel m_editNode;
    ResourceAccess m_resourceAccess;
    CubemapCreation m_cubemapCreation;
    ImGui::FileBrowser m_fileDialog;
    ImGuizmo::OPERATION m_imGuizmode = ImGuizmo::TRANSLATE;

    vec2 m_viewportSize { 0.0f };

    EditorMode m_mode;
    bool m_showAllNodes;
    bool m_disablePostProcessing;

    // Ref to nodes in Root
    std::shared_ptr<Node> m_rootEditorNode;
    std::shared_ptr<CameraNode> m_editorCamera;
    std::shared_ptr<Node> m_rootSceneNode;

    std::shared_ptr<Node> m_selectedNode;

    float m_cameraSpeed   = 0.15f;
    bool m_rightClickDown = false;
    bool m_leftClickDown = false;
    vec2 m_mouseDelta { 0.0f };

    bool m_pixelReadWaiting = false;
    unsigned int m_pixelRead = 0;

    void updateCamera();
    void updateImGuizmode();
    void displayMenuBar(bool validScene);
    void displayViewport(bool sceneSelected);
    void applicationWidgetRender();
};
}
