#pragma once

#include "Engine.hpp"

#include <imgui.h>
#include <algorithm>
#include <vector>

namespace Galaxy {
struct ResourceAccess {
    static std::unordered_map<Galaxy::uuid, std::string> paths;

    ResourceAccess() = default;
    ResourceAccess(const ResourceAccess&) = delete;
    ResourceAccess& operator=(const ResourceAccess&) = delete;

    uuid selectedResourceID = 0;
    std::string selectedResourcePath;

    // Use the same requester in show() and display() when sharing an instance.
    bool display(const std::string& requester = "")
    {
        if (!m_showResourceMenu || requester != m_requester)
            return false;
        const int frame = ImGui::GetFrameCount();
        if (m_lastDisplayFrame == frame)
            return false;
        m_lastDisplayFrame = frame;

        const std::string title = requester.empty() ? "Resources" : "Resources - " + requester;
        const std::string panelName = title + "###ResourceAccess_" + requester;
        const bool visible = ImGui::Begin(panelName.c_str(), &m_showResourceMenu);
        if (!visible || !m_showResourceMenu) {
            ImGui::End();
            return false;
        }
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

    void show(const std::string& resourceType = "", const std::string& requester = "")
    {
        show(resourceType.empty() ? std::vector<std::string> {} : std::vector<std::string> { resourceType }, requester);
    }

    void show(const std::vector<std::string>& resourceTypes, const std::string& requester = "")
    {
        m_visiblePaths.clear();
        for (const auto& path : Project::getPaths(ProjectPathTypes::RESOURCE)) {
            const auto type = resourceTypes.empty() ? std::string {} : IResourceMaker::readType(
                (std::filesystem::path(Project::getProjectRootPath()) / path.second).string());
            if (resourceTypes.empty() || std::find(resourceTypes.begin(), resourceTypes.end(), type) != resourceTypes.end())
                m_visiblePaths.insert(path);
        }
        selectedResourceID = 0;
        selectedResourcePath.clear();
        m_requester = requester;
        m_showResourceMenu = true;
    }

private:
    std::string m_requester;
    int m_lastDisplayFrame = -1;
    std::unordered_map<uuid, std::string> m_visiblePaths;
    bool m_showResourceMenu = false;
};
} // namespace Galaxy
