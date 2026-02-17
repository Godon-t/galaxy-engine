#include "EnvironmentNode.hpp"

#include "project/Project.hpp"
#include "rendering/renderer/Renderer.hpp"
#include "resource/ResourceManager.hpp"

namespace Galaxy {
EnvironmentNode::~EnvironmentNode()
{
    Renderer::getInstance().getBackend().clearCubemap(m_skyboxCubemapID);
}

void EnvironmentNode::accept(Galaxy::NodeVisitor& visitor)
{
    visitor.visit(*this);
}

inline void EnvironmentNode::draw()
{
    Node::draw();
    if (m_skyboxCubemapID != 0) {
        auto& frontend = Renderer::getInstance().getFrontend();
        frontend.changeUsedProgram(SKYBOX);
        frontend.submit(m_cubeMeshID, m_transform);
        frontend.changeUsedProgram(PBR);
    }
}

void EnvironmentNode::loadEnv(ResourceHandle<Environment> env)
{
    m_env = env;
    m_env.getResource().onLoaded([this] {
        auto& rendererInstance = Renderer::getInstance();
        m_skyboxCubemapID      = rendererInstance.getBackend().instanciateCubemap(m_env.getResource().getSkybox());

        rendererInstance.getFrontend().changeUsedProgram(SKYBOX);
        rendererInstance.getFrontend().useCubemap(m_skyboxCubemapID, "skybox");

        // Renderer::getInstance().renderFromPoint(vec3(0), *Application::getInstance().getRootNodePtr().get(), provisoryCubemap);
        // rendererInstance.applyFilterOnCubemap(m_cubeMeshID, m_skyboxCubemapID, m_irradianceCubemapID, FilterEnum::IRRADIANCE);

        // rendererInstance.changeUsedProgram(PBR);
        // rendererInstance.bindCubemap(m_irradianceCubemapID, "irradianceMap");
        // rendererInstance.setUniform("useIrradianceMap", true);
    });
}

void EnvironmentNode::enteredRoot()
{
    m_cubeMeshID          = Renderer::getInstance().getBackend().generateCube(999.f, true, [] {});
    m_irradianceCubemapID = Renderer::getInstance().getBackend().instanciateCubemap();
    m_renderingCubemap    = Renderer::getInstance().getBackend().instanciateCubemap();

    Renderer::getInstance().getFrontend().updateCubemap(m_renderingCubemap, 1024);
}

} // namespace Galaxy
