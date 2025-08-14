#pragma once

#include <Engine.hpp>
#include <imgui.h>

using namespace math;

namespace Galaxy {
class EditNode : public NodeVisitor {
public:
    void selectNode(Node& node)
    {
        ImGui::Begin("Node edit");
        node.accept(*this);
        ImGui::End();
    }
    void visit(Node& node)
    {
        ImGui::Text(std::to_string(node.id).c_str());
    }
    void visit(Node3D& node)
    {
        visit(static_cast<Node&>(node));
        transformEdit(*node.getTransform());
    }
    void visit(MeshInstance& node)
    {
        visit(static_cast<Node3D&>(node));
    }
    void visit(Camera& node)
    {
        visit(static_cast<Node3D&>(node));
    }
    void transformEdit(Transform& transform)
    {
        ImGui::SeparatorText("Transform");
        vec3 position = transform.getLocalPosition();
        if (ImGui::DragFloat3("Position", &position[0])) {
            transform.setLocalPosition(position);
        }
        vec3 rotation = transform.getLocalRotation();
        if (ImGui::DragFloat3("Rotation", &rotation[0])) {
            transform.setLocalRotation(rotation);
        }
        vec3 scale = transform.getLocalScale();
        if (ImGui::DragFloat3("Scale", &scale[0])) {
            transform.setLocalScale(scale);
        }
    }
};
}