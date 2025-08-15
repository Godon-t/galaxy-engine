#include "EditorLayer.hpp"
#include "editor/NodeList.hpp"
#include <Engine.hpp>

#include <imgui.h>

namespace Galaxy {
EditorLayer::~EditorLayer()
{
}

void EditorLayer::onAttach()
{
    // std::shared_ptr<MeshInstance> testInstance = std::make_shared<MeshInstance>();
    // testInstance->generateTriangle();
    // testInstance->translate(vec3(0, 0, 2));

    // std::shared_ptr<Camera> mainCam = std::make_shared<Camera>();
    // testInstance->addChild(mainCam);

    // m_selectedScene = Scene("TEST.glx");
    // m_selectedScene.load(testInstance);

    // Application::getInstance().setRootNode(testInstance);
    auto& renderer  = Renderer::getInstance();
    m_viewportFrame = new FrameBuffer(320, 180, FramebufferTextureFormat::RGBA8);
}

void EditorLayer::onDetach()
{
    m_viewportFrame->destroy();
}

void EditorLayer::onUpdate()
{
    m_viewportFrame->bind();
    if (m_selectedScene.getNodePtr()) {
        auto& renderer       = Renderer::getInstance();
        auto cameraTransform = CameraManager::getInstance().getCurrentCamTransform();
        renderer.beginSceneRender(cameraTransform);

        m_selectedScene.getNodePtr()->draw();

        renderer.endSceneRender();
        renderer.renderFrame();
    }
    m_viewportFrame->unbind();
}

void EditorLayer::onImGuiRender()
{
    ImGui::Begin("Application state");
    ImGui::Text((std::string("FPS: ") + std::to_string(1.0 / Application::getInstance().getDelta())).c_str());
    ImGui::End();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2 { 0, 0 });
    ImGui::Begin("Viewport");
    ImVec2 pannelSize = ImGui::GetContentRegionAvail();
    if (m_viewportSize != *(vec2*)&pannelSize) {
        m_viewportSize = { pannelSize.x, pannelSize.y };
        m_viewportFrame->resize(m_viewportSize.x, m_viewportSize.y);
    }
    ImGui::PopStyleVar();
    auto textureID = m_viewportFrame->getColorTextureID();
    ImGui::Image(reinterpret_cast<void*>(textureID), pannelSize, ImVec2 { 0, 0 }, ImVec2 { 1, 1 });
    ImGui::End();

    ImGui::Begin("Scene");

    if (m_selectedScene.isValid() && ImGui::Button("Serialize")) {
        m_selectedScene.save();
    }
    if (ImGui::Button("DeSerialize")) {
        if (!m_selectedScene.load("TEST.glx")) {
            GLX_CORE_ERROR("Failed to load '{0}'", "TEST.glx");
        } else {
            Application::getInstance().setRootNode(m_selectedScene.getNodePtr());
        }
    }

    auto rootPtr = Application::getInstance().getRootNodePtr();
    if (rootPtr) {
        m_nodeList.listNodes(*rootPtr);
    }
    if (m_nodeList.selectedNode) {
        m_editNode.selectNode(*m_nodeList.selectedNode);
    }
    ImGui::End();
}

void EditorLayer::onEvent(Event& evt)
{
    if (evt.isInCategory(EventCategory::EventCategoryApplication) && evt.getEventType() == EventType::WindowResize) {
        WindowResizeEvent& resizeEvt = (WindowResizeEvent&)evt;
        m_viewportFrame->resize(resizeEvt.getWidth(), resizeEvt.getHeight());
    }
}
}