#pragma once

#include "Engine.hpp"

#include <imgui.h>
#include "imfilebrowser.h"

#include <array>
#include <cstdio>

namespace Galaxy {
class CubemapCreation {
public:
    CubemapCreation()
    {
        std::snprintf(m_resourcePath.data(), m_resourcePath.size(), ".res/skybox.gres");
        m_fileDialog.SetTitle("Cubemap face image");
        m_fileDialog.SetTypeFilters({ ".*", ".png", ".jpg", ".jpeg", ".hdr", ".bmp", ".tga" });
    }

    void show()
    {
        m_show = true;
        m_importFailed = false;
        m_fileDialog.SetDirectory(Project::getProjectRootPath());
    }

    bool display()
    {
        if (!m_show)
            return false;

        bool imported = false;
        if (ImGui::Begin("Import cubemap", &m_show)) {
            static constexpr std::array<const char*, 6> labels {
                "+X (right)", "-X (left)", "+Y (top)", "-Y (bottom)", "+Z", "-Z"
            };
            for (std::size_t i = 0; i < m_faces.size(); ++i) {
                ImGui::PushID(static_cast<int>(i));
                if (ImGui::Button(labels[i])) {
                    m_faceIndex = i;
                    m_fileDialog.Open();
                }
                ImGui::SameLine();
                ImGui::TextUnformatted(m_faces[i].empty() ? "No image selected" : m_faces[i].c_str());
                ImGui::PopID();
            }
            ImGui::InputText("Resource path (.gres)", m_resourcePath.data(), m_resourcePath.size());
            ImGui::TextUnformatted("Use a path relative to the project. The resource references the selected images.");
            if (m_importFailed)
                ImGui::TextUnformatted("Import failed. Check the images and use an unused .gres resource path.");

            if (ImGui::Button("Import")) {
                imported = ResourceImporter::importCubemap(m_faces, m_resourcePath.data()) != 0;
                m_importFailed = !imported;
                if (imported)
                    m_show = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel"))
                m_show = false;
        }
        ImGui::End();

        if (!m_show) {
            m_fileDialog.Close();
            return imported;
        }
        m_fileDialog.Display();
        if (m_fileDialog.HasSelected()) {
            const auto selected = m_fileDialog.GetSelected();
            const auto relative = std::filesystem::relative(selected, Project::getProjectRootPath());
            m_faces[m_faceIndex] = relative.empty() ? selected.generic_string() : relative.generic_string();
            m_fileDialog.ClearSelected();
            m_importFailed = false;
        }
        return imported;
    }

private:
    CubemapResource::FacePaths m_faces;
    std::array<char, 1024> m_resourcePath {};
    ImGui::FileBrowser m_fileDialog { ImGuiFileBrowserFlags_EditPathString };
    std::size_t m_faceIndex = 0;
    bool m_show = false;
    bool m_importFailed = false;
};
} // namespace Galaxy
