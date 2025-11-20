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

        auto mathandle = node.getMaterial();
        if (mathandle && materialEdit(mathandle.getResource())) {
            Renderer::getInstance().updateMaterial(node.getMaterialId(), mathandle);
        }
        if (ImGui::Button("Save resource"))
            mathandle.getResource().save();
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

        ImGui::SeparatorText("Image");
        if (ImGui::Button("Load image"))
            m_resourceAccess.show();
        if (m_resourceAccess.display())
            node.loadTexture(m_resourceAccess.selectedResourcePath);
    }
    void visit(Camera& node)
    {
        visit(static_cast<Node3D&>(node));
    }
    void visit(EnvironmentNode& node)
    {
        visit(static_cast<Node&>(node));

        if (ImGui::Button("Load env")) {
            node.loadEnv(ResourceManager::getInstance().load<Environment>("env.gres"));
        }
        ImGui::SeparatorText("Testing");
        if (ImGui::Button("Render from center")) {
            node.testingFunc();
        }
    }
    void visit(SpotLight& node)
    {
        visit(static_cast<Node3D&>(node));

        ImGui::SeparatorText("SpotLight Properties");
        float intensity = node.getIntensity();
        if (ImGui::DragFloat("Intensity", &intensity, 0.01f, 0.0f, 10.0f)) {
            node.setIntensity(intensity);
        }
        vec3 color = node.getColor();
        if (ImGui::ColorEdit3("Color", &color[0])) {
            node.setColor(color);
        }

        float cutoffAngle = node.getCutoffAngle();
        if (ImGui::SliderFloat("Cutoff Angle", &cutoffAngle, 0.0f, 90.0f)) {
            node.setCutoffAngle(cutoffAngle);
        }

        float outerCutoffAngle = node.getOuterCutoffAngle();
        if (ImGui::SliderFloat("Outer Cutoff Angle", &outerCutoffAngle, 0.0f, 90.0f)) {
            node.setOuterCutoffAngle(outerCutoffAngle);
        }

        float range = node.getRange();
        if (ImGui::DragFloat("Range", &range, 0.1f, 0.1f, 100.0f)) {
            node.setRange(range);
        }

        bool castShadows = node.getCastShadows();
        if (ImGui::Checkbox("Cast Shadows", &castShadows)) {
            node.setCastShadows(castShadows);
        }
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

    bool materialEdit(Material& material)
    {
        ImGui::SeparatorText("Material");
        float transparencyVal = material.getTransparency();
        if (ImGui::SliderFloat("Transparency", &transparencyVal, 0.f, 1.f)) {
            material.setTransparency(transparencyVal);
            return true;
        }
        bool useTransparency = material.isUsingTransparency();
        if (ImGui::Checkbox("Use transparency", &useTransparency)) {
            material.setUseTransparency(useTransparency);
            return true;
        }
        return false;
    }

    ResourceAccess m_resourceAccess;
};
}