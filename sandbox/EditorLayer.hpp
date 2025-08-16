#pragma once

#include <Engine.hpp>

#include "editor/NodeEditPanel.hpp"
#include "editor/NodeListPanel.hpp"

namespace Galaxy {
enum class EditorMode {
    Edit,
    Run
};

class EditorLayer : public Layer {
public:
    EditorLayer()
        : Layer("Editor layer")
        , m_mode(EditorMode::Edit)
        , m_showAllNodes(false) {};
    ~EditorLayer() override;

    void onAttach() override;
    void onDetach() override;
    void onUpdate() override;
    void onImGuiRender() override;
    void onEvent(Event& evt) override;

private:
    Scene m_selectedScene;
    NodeListPanel m_nodeList;
    NodeEditPanel m_editNode;

    vec2 m_viewportSize;
    FrameBuffer* m_viewportFrame;

    EditorMode m_mode;
    bool m_showAllNodes;

    // Ref to nodes in Root
    std::shared_ptr<Node> m_rootEditorNode;
    std::shared_ptr<Camera> m_editorCamera;
    std::shared_ptr<Node> m_rootSceneNode;
};
}