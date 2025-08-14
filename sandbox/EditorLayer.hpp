#pragma once

#include <Engine.hpp>

#include "editor/EditNode.hpp"
#include "editor/NodeList.hpp"

namespace Galaxy {
class EditorLayer : public Layer {
public:
    EditorLayer()
        : Layer("Editor layer") {};
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
};
}