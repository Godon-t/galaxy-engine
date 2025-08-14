#include "ImGuiLayer.hpp"

#include <backend/imgui_impl_glfw.h>
#include <backend/imgui_impl_opengl3.h>
#include <imgui.h>

#include "engine/core/gl_headers.hpp"

#include "Application.hpp"
#include "Log.hpp"

#include "engine/event/MouseEvent.hpp"

namespace Galaxy {

ImGuiKey GlfwKeyToImGuiKey(int key)
{
    switch (key) {
    case GLFW_KEY_TAB:
        return ImGuiKey_Tab;
    case GLFW_KEY_LEFT:
        return ImGuiKey_LeftArrow;
    case GLFW_KEY_RIGHT:
        return ImGuiKey_RightArrow;
    case GLFW_KEY_UP:
        return ImGuiKey_UpArrow;
    case GLFW_KEY_DOWN:
        return ImGuiKey_DownArrow;
    case GLFW_KEY_PAGE_UP:
        return ImGuiKey_PageUp;
    case GLFW_KEY_PAGE_DOWN:
        return ImGuiKey_PageDown;
    case GLFW_KEY_HOME:
        return ImGuiKey_Home;
    case GLFW_KEY_END:
        return ImGuiKey_End;
    case GLFW_KEY_INSERT:
        return ImGuiKey_Insert;
    case GLFW_KEY_DELETE:
        return ImGuiKey_Delete;
    case GLFW_KEY_BACKSPACE:
        return ImGuiKey_Backspace;
    case GLFW_KEY_SPACE:
        return ImGuiKey_Space;
    case GLFW_KEY_ENTER:
        return ImGuiKey_Enter;
    case GLFW_KEY_ESCAPE:
        return ImGuiKey_Escape;
    case GLFW_KEY_KP_ENTER:
        return ImGuiKey_KeypadEnter;
    case GLFW_KEY_A:
        return ImGuiKey_A;
    case GLFW_KEY_C:
        return ImGuiKey_C;
    case GLFW_KEY_V:
        return ImGuiKey_V;
    case GLFW_KEY_X:
        return ImGuiKey_X;
    case GLFW_KEY_Y:
        return ImGuiKey_Y;
    case GLFW_KEY_Z:
        return ImGuiKey_Z;
    default:
        return ImGuiKey_None;
    }
}

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
    } else if (event.isInCategory(EventCategory::EventCategoryKeyboard)) {
        auto& io         = ImGui::GetIO();
        KeyEvent& keyEvt = (KeyEvent&)event;
        io.AddKeyEvent(GlfwKeyToImGuiKey(keyEvt.getKeyCode()), keyEvt.isPressed());
    } else if (event.isInCategory(EventCategory::EventCategoryChar)) {
        auto& io           = ImGui::GetIO();
        CharEvent& charEvt = (CharEvent&)event;
        io.AddInputCharacter(charEvt.getCodePoint());
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