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
    std::unique_ptr<MeshInstance> testInstance = std::make_unique<MeshInstance>();
    testInstance->generateTriangle();
    testInstance->translate(vec3(0, 0, 2));

    std::unique_ptr<Camera> mainCam = std::make_unique<Camera>();
    testInstance->addChild(std::move(mainCam));

    Application::getInstance().setRootNode(std::move(testInstance));
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

    if (ImGui::Button("Serialize")) {
        save(*Application::getInstance().getRootNodePtr(), m_scenePath);
    }

    NodeList nl;
    nl.listNodes(*Application::getInstance().getRootNodePtr());
    ImGui::End();
}

void EditorLayer::onEvent(Event& evt)
{
}

void EditorLayer::save(Node& node, std::string& filePath)
{
    NodeSerializer serializer;
    serializer.serialize(node, filePath.c_str());
}
}