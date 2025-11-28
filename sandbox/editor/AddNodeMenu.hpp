#pragma once

#include <Engine.hpp>
#include <imgui.h>

namespace Galaxy {
class AddNodeMenu {
private:
    const std::vector<std::string> m_nodeTypes = { "Node", "Node3D", "Camera", "MeshInstance", "MultiMeshInstance", "Sprite3D", "EnvironmentNode", "SpotLight", "PointLight", "GINode" };
    std::string m_selectedNode;

public:
    void open()
    {
        ImGui::OpenPopup("node_select");
    }
    bool display()
    {
        if (ImGui::BeginPopup("node_select")) {
            for (int i = 0; i < m_nodeTypes.size(); i++) {
                if (ImGui::Selectable(m_nodeTypes[i].c_str())) {
                    m_selectedNode = m_nodeTypes[i];
                    ImGui::EndPopup();
                    return true;
                }
            }
            ImGui::EndPopup();
        }
        return false;
    }
    std::string getSelectedNode() { return m_selectedNode; }
};

}