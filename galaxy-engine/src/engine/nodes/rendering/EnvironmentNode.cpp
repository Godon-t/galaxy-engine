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
    if (updateRendering) {
        updateRendering        = false;
        auto& rendererInstance = Renderer::getInstance();
        rendererInstance.renderFromPoint(vec3(0), *Application::getInstance().getRootNodePtr().get(), m_renderingCubemap);
        rendererInstance.beginCanvaNoBuffer();
        rendererInstance.debugMessage("Binding after rendering from point");
        rendererInstance.changeUsedProgram(SKYBOX);
        rendererInstance.useCubemap(m_renderingCubemap, "skybox");
        rendererInstance.endCanva();

    } else if (m_skyboxCubemapID != 0) {
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
        rendererInstance.useCubemap(m_skyboxCubemapID, "skybox");

        // Renderer::getInstance().renderFromPoint(vec3(0), *Application::getInstance().getRootNodePtr().get(), provisoryCubemap);
        // rendererInstance.applyFilterOnCubemap(m_cubeMeshID, m_skyboxCubemapID, m_irradianceCubemapID, FilterEnum::IRRADIANCE);

        // rendererInstance.changeUsedProgram(PBR);
        // rendererInstance.bindCubemap(m_irradianceCubemapID, "irradianceMap");
        // rendererInstance.setUniform("useIrradianceMap", true);
    });
}

void EnvironmentNode::enteredRoot()
{
    m_cubeMeshID          = Renderer::getInstance().generateCube(9999.f, true, [] {});
    m_irradianceCubemapID = Renderer::getInstance().instanciateCubemap();
    m_renderingCubemap    = Renderer::getInstance().instanciateCubemap();

    Renderer::getInstance().resizeCubemap(m_renderingCubemap, 1024);
    updateRendering = false;
}

} // namespace Galaxy
