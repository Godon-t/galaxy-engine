#pragma once

#include "Engine.hpp"

#include <imgui.h>

namespace Galaxy {
struct ResourceAccess {
    uuid selectedResourceID;
    std::string selectedResourcePath;

    bool chooseResource()
    {
        ImGui::Begin("Resources");
        auto paths = Project::getPaths(ProjectPathTypes::RESOURCE);
        for (auto& path : paths) {
            if (ImGui::MenuItem(path.second.c_str())) {
                selectedResourceID   = path.first;
                selectedResourcePath = path.second;
                ImGui::End();
                return true;
            }
        }
        ImGui::End();
        return false;
    }
};
} // namespace Galaxy
