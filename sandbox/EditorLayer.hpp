#pragma once

#include <Engine.hpp>

namespace Galaxy {
class EditorLayer : public Layer {
public:
    EditorLayer()
        : Layer("Editor layer")
        , m_scenePath("TEST.glx") {};
    ~EditorLayer() override;

    void onAttach() override;
    void onDetach() override;
    void onUpdate() override;
    void onImGuiRender() override;
    void onEvent(Event& evt) override;

private:
    std::string m_scenePath;
    void save(Node& node, std::string& filePath);
};
}