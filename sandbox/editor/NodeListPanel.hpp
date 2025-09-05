#pragma once

#include <Engine.hpp>
#include <imgui.h>

#include "AddNodeMenu.hpp"

namespace Galaxy {
class NodeListPanel : public NodeVisitor {
public:
    size_t selectedNodeId;

    void listNodes(Node& root)
    {
        root.accept(*this);
    }

    void visit(Node& node) override
    {
        ImGui::PushID(node.id);
        if (node.getChildCount() == 0) {
            ImGui::BulletText(node.getName().c_str());
            editHierarchyMenu(node);
        } else {
            bool treeNotColapse = ImGui::TreeNode(node.getName().c_str());
            editHierarchyMenu(node);
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
    void visit(MultiMeshInstance& node) override
    {
        visit(static_cast<Node3D&>(node));
    }
    void visit(Sprite3D& node) override
    {
        visit(static_cast<Node3D&>(node));
    }

private:
    AddNodeMenu m_addNodeMenu;

    void editHierarchyMenu(Node& node)
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
        if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            selectedNodeId = node.id;
        }
    }
};
}