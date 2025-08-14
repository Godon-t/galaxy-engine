#pragma once

#include <Engine.hpp>
#include <imgui.h>

#include "AddNodeMenu.hpp"

namespace Galaxy {
class NodeList : public NodeVisitor {
public:
    void listNodes(Node& root)
    {
        root.accept(*this);
    }

    void visit(Node& node) override
    {
        if (node.getChildCount() == 0) {
            ImGui::BulletText(node.getName().c_str());
            ImGui::SameLine();
            ImGui::PushID(node.id);
            if (ImGui::Button("Add node")) {
                m_addNodeMenu.open();
            }
            ImGui::PopID();
        } else {
            bool treeNotColapse = ImGui::TreeNode(node.getName().c_str());
            ImGui::SameLine();
            ImGui::PushID(node.id);
            if (ImGui::Button("Add node")) {
                m_addNodeMenu.open();
            }
            ImGui::PopID();
            if (treeNotColapse) {
                for (auto child : node.getChildren()) {
                    child->accept(*this);
                }
                ImGui::TreePop();
            }
        }
        if (m_addNodeMenu.display()) {
            auto nodeType                 = m_addNodeMenu.getSelectedNode();
            std::shared_ptr<Node> newNode = constructNode(nodeType);
            node.addChild(newNode);
        }
    }
    void visit(Node3D& node) override
    {
        visit(static_cast<Node&>(node));
    }
    void visit(Camera& node) override
    {
        visit(static_cast<Node3D&>(node));
    }
    void visit(MeshInstance& node) override
    {
        visit(static_cast<Node3D&>(node));
    }

private:
    AddNodeMenu m_addNodeMenu;
};
}