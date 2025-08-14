#include "ImGuiLayer.hpp"

#include <backend/imgui_impl_glfw.h>
#include <backend/imgui_impl_opengl3.h>
#include <imgui.h>

#include "engine/core/gl_headers.hpp"

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
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    GLFWwindow* window = static_cast<GLFWwindow*>(Application::getInstance().getWindow().getNativeWindow());
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");
}

void ImGuiLayer::onDetach()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
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
}

void ImGuiLayer::begin()
{
    auto& io       = ImGui::GetIO();
    auto& window   = Application::getInstance().getWindow();
    io.DisplaySize = ImVec2(window.getWidth(), window.getHeight());
    ImGui_ImplGlfw_NewFrame();
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