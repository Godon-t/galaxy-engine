#pragma once

#include <Engine.hpp>
#include <imgui.h>

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
        } else if (ImGui::TreeNode(node.getName().c_str())) {
            for (auto child : node.getChildren()) {
                child->accept(*this);
            }
            ImGui::TreePop();
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
};
}