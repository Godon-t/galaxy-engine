#include "ImGuiLayer.hpp"

#include <backend/imgui_impl_glfw.h>
#include <backend/imgui_impl_opengl3.h>
#include <imgui.h>

#include "Application.hpp"
#include "Log.hpp"

#include "engine/event/MouseEvent.hpp"

namespace Galaxy {
ImGuiLayer::~ImGuiLayer()
{
}

void ImGuiLayer::onAttach()
{
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    auto& io = ImGui::GetIO();
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
    // map io keys to glfw keys (io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB)

    ImGui_ImplOpenGL3_Init("#version 410");
}

void ImGuiLayer::onDetach()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiLayer::onUpdate()
{
}

void ImGuiLayer::onImGuiRender()
{
}

void ImGuiLayer::onEvent(Event& event)
{
    if (event.isInCategory(EventCategory::EventCategoryMouse)) {
        auto& io = ImGui::GetIO();
        if (event.getEventType() == EventType::MouseButtonInteract) {
            MouseButtonEvent& mousButton = (MouseButtonEvent&)event;
            if (mousButton.getButton() == MOUSE_LEFT)
                io.MouseDown[0] = mousButton.isPressed();
            else if (mousButton.getButton() == MOUSE_RIGHT)
                io.MouseDown[1] = mousButton.isPressed();
        } else if (event.getEventType() == EventType::MouseMotion) {
            MouseMotionEvent& mousMotion = (MouseMotionEvent&)event;
            io.MousePos                  = ImVec2(mousMotion.getX(), mousMotion.getY());
        }
    }
}

void ImGuiLayer::begin()
{
    auto& io       = ImGui::GetIO();
    auto& window   = Application::getInstance().getWindow();
    io.DisplaySize = ImVec2(window.getWidth(), window.getHeight());
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Scene", nullptr);
}

void ImGuiLayer::end()
{
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
}