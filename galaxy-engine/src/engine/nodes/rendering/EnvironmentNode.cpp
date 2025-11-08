#include "EnvironmentNode.hpp"

#include "project/Project.hpp"
#include "rendering/renderer/Renderer.hpp"
#include "resource/ResourceManager.hpp"

namespace Galaxy {
EnvironmentNode::~EnvironmentNode()
{
    Renderer::getInstance().clearCubemap(m_skyboxCubemapID);
}

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

void EnvironmentNode::loadEnv(ResourceHandle<Environment> env)
{
    m_env = env;
    m_env.getResource().onLoaded([this] {
        auto& rendererInstance = Renderer::getInstance();
        m_skyboxCubemapID      = rendererInstance.instanciateCubemap(m_env.getResource().getSkybox());
        m_irradianceCubemapID  = rendererInstance.instanciateCubemap();
        rendererInstance.renderFromPoint(m_transform.getGlobalPosition(), *Application::getInstance().getRootNodePtr().get(), m_irradianceCubemapID);
        // m_env.getResource().m_skyboxCubemapID = m_skyboxCubemapID;
    });
}

void EnvironmentNode::enteredRoot()
{
    // loadEnv(ResourceManager::getInstance().load<Environment>("env.gres"));

    m_cubeMeshID = Renderer::getInstance().generateCube(9998.f, true, [] {});
}

} // namespace Galaxy
