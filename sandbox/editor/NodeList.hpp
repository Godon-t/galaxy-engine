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
        ImGui::PushID(node.id);
        if (node.getChildCount() == 0) {
            ImGui::BulletText(node.getName().c_str());
            addEditButtons(node);
        } else {
            bool treeNotColapse = ImGui::TreeNode(node.getName().c_str());
            addEditButtons(node);
            if (treeNotColapse) {
                for (auto child : node.getChildren()) {
                    child->accept(*this);
                }
                ImGui::TreePop();
            }
        }
        ImGui::PopID();
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
    void addEditButtons(Node& node)
    {
        if (ImGui::BeginPopupContextItem("Edit node")) {
            if (ImGui::Button("Add node")) {
                m_addNodeMenu.open();
            }
            if (m_addNodeMenu.display()) {
                auto nodeType                 = m_addNodeMenu.getSelectedNode();
                std::shared_ptr<Node> newNode = constructNode(nodeType);
                node.addChild(newNode);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::Selectable("Delete")) {
                node.destroy();
            }
            ImGui::EndPopup();
        }
    }
};
}