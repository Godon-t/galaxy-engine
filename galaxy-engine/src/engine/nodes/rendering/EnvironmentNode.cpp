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
    Node::draw();
    if (m_skyboxCubemapID != 0) {
        auto& ri = Renderer::getInstance();
        ri.changeUsedProgram(SKYBOX);
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

        rendererInstance.changeUsedProgram(SKYBOX);
        rendererInstance.bindCubemap(m_skyboxCubemapID, "skybox");

        // Renderer::getInstance().renderFromPoint(vec3(0), *Application::getInstance().getRootNodePtr().get(), provisoryCubemap);
        rendererInstance.applyFilterOnCubemap(m_cubeMeshID, m_skyboxCubemapID, m_irradianceCubemapID, FilterEnum::IRRADIANCE);

        rendererInstance.changeUsedProgram(PBR);
        rendererInstance.bindCubemap(m_irradianceCubemapID, "irradianceMap");
        // rendererInstance.setUniform("useIrradianceMap", true);
    });
}

void EnvironmentNode::enteredRoot()
{
    // loadEnv(ResourceManager::getInstance().load<Environment>("env.gres"));

    m_cubeMeshID          = Renderer::getInstance().generateCube(9999.f, true, [] {});
    m_irradianceCubemapID = Renderer::getInstance().instanciateCubemap();
}

} // namespace Galaxy
