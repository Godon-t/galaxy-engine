#include "EditorLayer.hpp"

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
    ImGui::Text("Salut depuis l'ui d'editor!");
}

void EditorLayer::onEvent(Event& evt)
{
}
}