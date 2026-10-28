#pragma once

#include "Engine.hpp"
#include "ResourceAccess.hpp"

#include <imgui.h>

using namespace math;

namespace Galaxy {
    const BitMask editorVisibility {2};

class NodeEditPanel : public NodeVisitor {
public:
    void init()
    {
        auto& backend = Renderer::getInstance().getBackend();
        m_visualBuffer = backend.instantiateFrameBuffer(512, 512, FramebufferTextureFormat::DEPTH24RGBA8);
        m_invertedCube = backend.generateCube(1, true, [] {});
        m_cubemapMaterialResource = ResourceManager::getInstance().createInternalResource<TextureMaterial>();
        m_cubemapMaterial = backend.instantiateMaterial(m_cubemapMaterialResource);
    }

    void destroy()
    {
        auto& backend = Renderer::getInstance().getBackend();
        backend.clearMaterial(m_cubemapMaterial);
        backend.clearMesh(m_invertedCube);
        backend.clearFrameBuffer(m_visualBuffer);
    }
    void selectNode(Node& node, const vec2& mouseDelta = vec2(0.0f))
    {
        m_mouseDelta = mouseDelta;
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
        transformEdit(node.getTransform());
    }
    void visit(MeshInstance& node)
    {
        visit(static_cast<Node3D&>(node));

        ImGui::SeparatorText("Mesh resource");

        if (ImGui::Button("Load mesh"))
            m_resourceAccess.show();
        if (m_resourceAccess.display())
            node.loadMesh(m_resourceAccess.selectedResourcePath);

        auto mathandle = node.getMaterial();
        if (mathandle && materialEdit(mathandle.getResource())) {
            Renderer::getInstance().getBackend().updateMaterial(node.getMaterialId(), mathandle);
        }
        if (ImGui::Button("Save resource"))
            mathandle.getResource().save();
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
    void visit(CameraNode& node)
    {
        visit(static_cast<Node3D&>(node));

        bool state = node.getCurrent();
        if(ImGui::Checkbox("current", &state))
            node.setCurrent(state);
        
        static FramebufferHandle cameraFramebuffer = Renderer::getInstance().getBackend().instantiateFrameBuffer(512, 256, FramebufferTextureFormat::DEPTH24RGBA8);
        
        Camera camera = *node.getCamera();
        camera.viewportSize = vec2(512, 256);
        camera.targetFramebuffer = cameraFramebuffer;
        Renderer::getInstance().getFrontend().addCamera(std::move(camera));
        
        
        auto textureID = Renderer::getInstance().getBackend().getFrameBufferTextureID(cameraFramebuffer);
        GLX_TRACE("textureID: {0}", textureID);
        ImVec2 pannelSize(512, 256);
        ImGui::Image(reinterpret_cast<void*>(textureID), pannelSize, ImVec2 { 1, 1 }, ImVec2 { 0, 0 });
    }
    void visit(EnvironmentNode& node)
    {
        visit(static_cast<Node&>(node));

        if (ImGui::Button("Load env")) {
            node.loadEnv(ResourceManager::getInstance().load<Environment>("env.gres"));
        }
    }
    void visit(Light& node)
    {
        visit(static_cast<Node3D&>(node));

        ImGui::SeparatorText("Light Properties");
        float intensity = node.getIntensity();
        if (ImGui::DragFloat("Intensity", &intensity, 0.01f, 0.0f, 10.0f)) {
            node.setIntensity(intensity);
        }
        vec3 color = node.getColor();
        if (ImGui::ColorEdit3("Color", &color[0])) {
            node.setColor(color);
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
    void visit(SpotLight& node)
    {
        visit(static_cast<Light&>(node));

        ImGui::SeparatorText("SpotLight Properties");
        float innerCutoffAngle = node.getInnerCutoffAngle();
        if (ImGui::SliderFloat("Inner Cutoff Angle", &innerCutoffAngle, 0.0f, 45.0f)) {
            node.setInnerCutoffAngle(innerCutoffAngle);
        }
        
        float outerCutoffAngle = node.getOuterCutoffAngle();
        if (ImGui::SliderFloat("Outer Cutoff Angle", &outerCutoffAngle, 0.0f, 45.0f)) {
            node.setOuterCutoffAngle(outerCutoffAngle);
        }
    }

    void visit(PointLight& node) override
    {
        visit(static_cast<Light&>(node));
    }

    void visit(GINode& node)
    {
        visit(static_cast<Node3D&>(node));
        ivec3 dimmensions  = node.getGrid();
        float spaceBetween = node.getSpaceBetween();
        int res            = node.getProbeResolution();

        ImGui::SeparatorText("Global Illumination");
        if (ImGui::Button("Update probes")) {
            node.updateProbes();
        }

        if (ImGui::InputInt3("Probe grid dimmensions", &dimmensions.x))
            node.setGrid(dimmensions);
        if (ImGui::InputFloat("Space between probes", &spaceBetween))
            node.setSpaceBetween(spaceBetween);
        if (ImGui::InputInt("Probe texture resolution", &res))
            node.setProbeResolution(res);
        // if (ImGui::InputFloat("Probe bias", &bias))
        //     Renderer::getInstance().getLightManager().updateBias(bias);
        ImGui::Checkbox("Debug", &node.debugMode);
    }

    void visit(ProbeNode& node) override
    {
        visit(static_cast<Node3D&>(node));

        ImGui::SeparatorText("Probe");
        if (ImGui::Button("Capture cubemap"))
            node.capture();

        if (!node.hasCapture())
            return;

        auto& renderer = Renderer::getInstance();
        auto& material = m_cubemapMaterialResource.getResource();
        const auto* cubemap = std::get_if<CubemapHandle>(&material.getTexture());
        if (!cubemap || *cubemap != node.getCubemapHandle()) {
            material.setTexture(node.getCubemapHandle());
            renderer.getBackend().updateMaterial(m_cubemapMaterial, m_cubemapMaterialResource);
        }

        const ImVec2 imagePosition = ImGui::GetCursorScreenPos();
        const ImVec2 imageSize(256, 256);
        ImGui::Image(reinterpret_cast<void*>(renderer.getBackend().getFrameBufferTextureID(m_visualBuffer)),
            imageSize, ImVec2 { 1, 1 }, ImVec2 { 0, 0 });
        ImGui::SetCursorScreenPos(imagePosition);
        ImGui::InvisibleButton("##CubemapRotation", imageSize, ImGuiButtonFlags_MouseButtonLeft);
        if (ImGui::IsItemHovered() || ImGui::IsItemActive())
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        if (ImGui::IsItemActive()) {
            m_cubemapTransform.localRotateY(m_mouseDelta.x * -0.005f);
            m_cubemapTransform.globalRotateX(m_mouseDelta.y * 0.005f);
            m_cubemapTransform.computeModelMatrix();
        }

        Camera camera;
        camera.position = vec3(0, 0, 2.5f);
        camera.forward = vec3(0, 0, -1);
        camera.right = vec3(-1, 0, 0);
        camera.viewportSize = vec2(512);
        camera.visibilityMask = editorVisibility;
        camera.targetFramebuffer = m_visualBuffer;
        renderer.getFrontend().addCamera(std::move(camera));

        renderer.getFrontend().addObjectToScene(m_invertedCube, m_cubemapMaterial, m_cubemapTransform, editorVisibility);
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

    bool materialEdit(BaseMaterial& material)
    {
        ImGui::SeparatorText("Material");
        bool changed = false;
        float transparencyVal = material.getTransparency();
        if (ImGui::SliderFloat("Transparency", &transparencyVal, 0.f, 1.f)) {
            material.setTransparency(transparencyVal);
            changed = true;
        }
        bool useTransparency = material.isUsingTransparency();
        if (ImGui::Checkbox("Use transparency", &useTransparency)) {
            material.setUseTransparency(useTransparency);
            changed = true;
        }

        if (auto* pbrMaterial = dynamic_cast<PBRMaterial*>(&material)) {
            ImGui::SeparatorText("PBR");
            vec3 albedo = pbrMaterial->getAlbedo();
            if (ImGui::ColorEdit3("Albedo", &albedo[0])) {
                pbrMaterial->setAlbedo(albedo);
                changed = true;
            }
            float metallic = pbrMaterial->getMetallic();
            if (ImGui::SliderFloat("Metallic", &metallic, 0.f, 1.f)) {
                pbrMaterial->setMetallic(metallic);
                changed = true;
            }
            float roughness = pbrMaterial->getRoughness();
            if (ImGui::SliderFloat("Roughness", &roughness, 0.f, 1.f)) {
                pbrMaterial->setRoughness(roughness);
                changed = true;
            }
            float ambient = pbrMaterial->getAmbient();
            if (ImGui::SliderFloat("Ambient", &ambient, 0.f, 1.f)) {
                pbrMaterial->setAmbient(ambient);
                changed = true;
            }
        } else if (auto* textureMaterial = dynamic_cast<TextureMaterial*>(&material)) {
            ImGui::SeparatorText("Texture");
            const auto& source = textureMaterial->getTexture();
            if (!textureMaterial->hasTexture()) {
                ImGui::TextUnformatted("No texture");
            } else if (const auto* image = std::get_if<ResourceHandle<Image>>(&source)) {
                ImGui::Text("Image: %s", image->getResource().getPath().c_str());
            } else if (std::holds_alternative<CubemapHandle>(source)) {
                ImGui::TextUnformatted("Cubemap");
            } else {
                ImGui::TextUnformatted("Texture 2D");
            }

            if (ImGui::Button("Load image"))
                m_textureResourceAccess.show();
            if (m_textureResourceAccess.display()) {
                textureMaterial->setTexture(ResourceManager::getInstance().load<Image>(m_textureResourceAccess.selectedResourcePath));
                changed = true;
            }
        }

        return changed;
    }

    ResourceAccess m_resourceAccess;
    ResourceAccess m_textureResourceAccess {};
    FramebufferHandle m_visualBuffer;
    GeometryHandle m_invertedCube;
    ResourceHandle<TextureMaterial> m_cubemapMaterialResource;
    MaterialHandle m_cubemapMaterial;
    Transform m_cubemapTransform;
    vec2 m_mouseDelta { 0.0f };
};
}
