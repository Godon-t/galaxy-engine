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
}

void EditorLayer::onDetach()
{
}

void EditorLayer::onUpdate()
{
}

void EditorLayer::onImGuiRender()
{
    ImGui::Begin("Application state");
    ImGui::Text((std::string("FPS: ") + std::to_string(1.0 / Application::getInstance().getDelta())).c_str());
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
    ImGui::End();
}

void EditorLayer::onEvent(Event& evt)
{
}
}