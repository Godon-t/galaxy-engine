#include "EditorLayer.hpp"

#include <imgui.h>

namespace Galaxy {
EditorLayer::~EditorLayer()
{
}

void EditorLayer::onAttach()
{
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