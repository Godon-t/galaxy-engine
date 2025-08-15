#pragma once

#include <Engine.hpp>

#include "editor/EditNode.hpp"
#include "editor/NodeList.hpp"

namespace Galaxy {
enum class EditorMode {
    Edit,
    Run
};

class EditorLayer : public Layer {
public:
    EditorLayer()
        : Layer("Editor layer")
        , m_mode(EditorMode::Edit) {};
    ~EditorLayer() override;

    void onAttach() override;
    void onDetach() override;
    void onUpdate() override;
    void onImGuiRender() override;
    void onEvent(Event& evt) override;

private:
    Scene m_selectedScene;
    NodeList m_nodeList;
    EditNode m_editNode;

    vec2 m_viewportSize;
    FrameBuffer* m_viewportFrame;

    EditorMode m_mode;
};
}