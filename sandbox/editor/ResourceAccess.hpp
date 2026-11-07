#pragma once

#include "Engine.hpp"

#include <imgui.h>

namespace Galaxy {
struct ResourceAccess {
    static std::unordered_map<Galaxy::uuid, std::string> paths;

    uuid selectedResourceID;
    std::string selectedResourcePath;

    bool display()
    {
        if (!m_showResourceMenu)
            return false;

        ImGui::Begin("Resources");
        if (ImGui::Button("Close")) {
            ImGui::End();
            m_showResourceMenu = false;
            return false;
        }
        for (auto& path : m_visiblePaths) {
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

    void show(const std::string& resourceType = "")
    {
        m_visiblePaths.clear();
        for (const auto& path : Project::getPaths(ProjectPathTypes::RESOURCE)) {
            if (resourceType.empty() || IResourceMaker::readType(
                    (std::filesystem::path(Project::getProjectRootPath()) / path.second).string()) == resourceType)
                m_visiblePaths.insert(path);
        }
        m_showResourceMenu = true;
    }

private:
    std::unordered_map<uuid, std::string> m_visiblePaths;
    bool m_showResourceMenu = false;
};
} // namespace Galaxy
