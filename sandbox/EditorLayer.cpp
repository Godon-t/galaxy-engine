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
    std::shared_ptr<MeshInstance> testInstance = std::make_shared<MeshInstance>();
    testInstance->generateTriangle();
    testInstance->translate(vec3(0, 0, 2));

    std::unique_ptr<Camera> mainCam = std::make_unique<Camera>();
    testInstance->addChild(std::move(mainCam));

    m_selectedScene = Scene("TEST.glx");
    m_selectedScene.load(testInstance);

    Application::getInstance().setRootNode(testInstance);
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

    NodeList nl;
    nl.listNodes(*Application::getInstance().getRootNodePtr());
    ImGui::End();
}

void EditorLayer::onEvent(Event& evt)
{
}
}