#include "Engine.hpp"
#include "ResourceAccess.hpp"

#include <imgui.h>

namespace Galaxy {
class EnvironmentCreation {
public:
    EnvironmentCreation()
        : m_path("env.gres")
    {
    }
    void show()
    {
        m_show = true;
    }
    bool display()
    {
        if (!m_show)
            return false;

        ImGui::Begin("Environment creation");

        auto imageButton = [this](char* name, int imageIdx) {
            if (ImGui::Button(name)) {
                m_resourceAccess.show();
                m_pathIdx = imageIdx;
            }
        };

        imageButton("Image1 path", 0);
        imageButton("Image2 path", 1);
        imageButton("Image3 path", 2);
        imageButton("Image4 path", 3);
        imageButton("Image5 path", 4);
        imageButton("Image6 path", 5);

        if (m_resourceAccess.display()) {
            m_skyboxPaths[m_pathIdx] = m_resourceAccess.selectedResourcePath;
            m_skyboxIDs[m_pathIdx]   = m_resourceAccess.selectedResourceID;
        }

        if (ImGui::Button("Create")) {
            std::array<ResourceHandle<Image>, 6> images;
            for (int i = 0; i < 6; i++) {
                images[i] = ResourceManager::getInstance().load<Image>(m_skyboxPaths[i]);
            }
            Environment env { images };
            auto envResource = ResourceManager::getInstance().registerNewResource<Environment>(env, m_path);
            envResource.getResource().save(false);
            ImGui::End();
            m_show = false;
            return true;
        }
        ImGui::End();
        return false;
    }
    void clear()
    {
        std::string erase("");
        m_skyboxPaths.fill(erase);
        m_skyboxIDs.fill(0);
        m_show    = false;
        m_pathIdx = -1;
    }

private:
    std::array<std::string, 6> m_skyboxPaths;
    std::array<uuid, 6> m_skyboxIDs;
    std::string m_path;
    bool m_show = false;
    ResourceAccess m_resourceAccess;
    int m_pathIdx = -1;
};
} // namespace Galaxy
