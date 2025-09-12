#pragma once

#include "Engine.hpp"
#include "ResourceAccess.hpp"

#include <imgui.h>

using namespace math;

namespace Galaxy {
class NodeEditPanel : public NodeVisitor {
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

        ImGui::SeparatorText("Mesh resource");

        if (ImGui::Button("Load mesh"))
            m_resourceAccess.show();
        if (m_resourceAccess.display()) {
            auto meshRes = ResourceManager::getInstance().load<Mesh>(m_resourceAccess.selectedResourcePath);
            node.loadMesh(meshRes, 0);
        }

        int surfaceIdx = node.getSurfaceIdx();
        if (ImGui::InputInt("Surface idx", &surfaceIdx)) {
            node.loadMesh(node.getMeshResource(), surfaceIdx);
        }
    }
    void visit(MultiMeshInstance& node)
    {
        visit(static_cast<Node3D&>(node));

        if (ImGui::Button("Load mesh"))
            m_resourceAccess.show();
        if (m_resourceAccess.display())
            node.loadMesh(m_resourceAccess.selectedResourcePath);
    }
    void visit(Sprite3D& node)
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
        vec3 startRotation    = transform.getLocalRotation() / (M_PI / 180.f);
        vec3 modifiedRotation = startRotation;
        if (ImGui::DragFloat3("Rotation", &modifiedRotation[0], 0.1f)) {
            // transform.rotate((modifiedRotation - startRotation) * (M_PI / 180.f));
            transform.setLocalRotation(modifiedRotation * (M_PI / 180.f));
        }
        vec3 scale = transform.getLocalScale();
        if (ImGui::DragFloat3("Scale", &scale[0])) {
            transform.setLocalScale(scale);
        }
    }

    ResourceAccess m_resourceAccess;
};
}