#pragma once

#include "Engine.hpp"

#include <imgui.h>

namespace Galaxy {
struct ResourceAccess {
    static std::unordered_map<Galaxy::uuid, std::string> paths;

    uuid selectedResourceID;
    std::string selectedResourcePath;

    bool begin()
    {
        if (!m_showResourceMenu)
            return false;

        ImGui::Begin("Resources");
        if (ImGui::Button("Close")) {
            ImGui::End();
            m_showResourceMenu = false;
            return false;
        }
        for (auto& path : paths) {
            if (ImGui::MenuItem(path.second.c_str())) {
                selectedResourceID   = path.first;
                selectedResourcePath = path.second;
                m_showResourceMenu   = false;
                ImGui::End();
                return true;
            }
        }
        ImGui::End();
        return false;
    }

    void show()
    {
        m_showResourceMenu = true;
    }

private:
    bool m_showResourceMenu;
};
} // namespace Galaxy
