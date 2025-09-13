#include "EnvironmentNode.hpp"

#include "project/Project.hpp"
#include "rendering/renderer/Renderer.hpp"
#include "resource/ResourceManager.hpp"

namespace Galaxy {
void EnvironmentNode::accept(Galaxy::NodeVisitor& visitor)
{
    visitor.visit(*this);
}

inline void EnvironmentNode::draw()
{
    if (m_skyboxCubemapID != 0) {
        auto& ri = Renderer::getInstance();
        ri.changeUsedProgram(SKYBOX);
        ri.bindCubemap(m_skyboxCubemapID, "skybox");
        ri.submit(m_cubeMeshID, m_transform);
        ri.changeUsedProgram(PBR);
    }
}

void EnvironmentNode::enteredRoot()
{
    m_env = ResourceManager::getInstance().load<Environment>("env.gres");
    m_env.getResource().onLoaded([this] {
        m_skyboxCubemapID = Renderer::getInstance().instanciateCubemap(m_env.getResource().getSkybox());
        // m_env.getResource().m_skyboxCubemapID = m_skyboxCubemapID;
    });

    m_cubeMeshID = Renderer::getInstance().generateCube(999.f, true, [] {});
}

} // namespace Galaxy
