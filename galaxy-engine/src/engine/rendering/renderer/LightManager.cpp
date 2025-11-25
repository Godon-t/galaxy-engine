#include "LightManager.hpp"
#include "core/Application.hpp"
#include "engine/rendering/CameraManager.hpp"
#include "engine/rendering/renderer/Renderer.hpp"

namespace Galaxy {
LightManager::LightManager()
    : m_shadowMapFrameBufferID(0)
    , m_probesFrameBuffer(0)
    , m_renderingCubemap(0)
    , m_fullQuad(0)
{
}

LightManager::~LightManager()
{
}

void LightManager::init()
{
    auto& ri                 = Renderer::getInstance();
    m_shadowMapFrameBufferID = ri.instanciateFrameBuffer(1024, 1024, FramebufferTextureFormat::DEPTH);

    m_fullQuad = ri.generateQuad(vec2(2, 2), []() {});

    m_renderingCubemap = ri.instanciateCubemap();
    ri.resizeCubemap(m_renderingCubemap, 1024);

    // TODO: pass to a format for normals in addition to colors and depths
    m_probesFrameBuffer = ri.instanciateFrameBuffer(1024, 1024, FramebufferTextureFormat::DEPTH24RGBA8);

    ri.beginCanvaNoBuffer();
    ri.attachTextureToColorFramebuffer(m_probeRadianceTexture, m_probesFrameBuffer);
    ri.endCanva();
}

int LightManager::registerLight(const SpotLight* desc)
{
    lightID id = m_nextLightID++;
    int idx    = m_currentLightCount++;

    math::mat4 lightTransform = desc->getTransform()->getGlobalModelMatrix(); // Use
    m_lights[id]              = LightData(idx, lightTransform);

    renderID shadowMapID     = Renderer::getInstance().instantiateTexture();
    m_lights[id].shadowMapID = shadowMapID;
    return id;
}

void LightManager::updateLightTransform(lightID id, math::mat4 transform)
{
    m_lights[id].transformationMatrix = transform;
}

void LightManager::unregisterLight(int id)
{
    m_lights.erase(id);
}

void LightManager::shadowPass(Node* sceneRoot)
{
    auto& ri = Renderer::getInstance();

    math::mat4 projMat  = CameraManager::processProjectionMatrix(vec2(1024, 1024));
    int currentLightIdx = 0;
    for (auto& [id, lightData] : m_lights) {
        if (currentLightIdx >= m_maxLights)
            break;

        math::mat4 view             = CameraManager::processViewMatrix(lightData.transformationMatrix);
        math::mat4 lightSpaceMatrix = projMat * view;

        ri.beginCanva(view, projMat, m_shadowMapFrameBufferID, FramebufferTextureFormat::DEPTH);
        ri.linkCanvaDepthToTexture(lightData.shadowMapID);

        ri.setUniform("lights[" + std::to_string(id) + "].lightMatrix", lightSpaceMatrix);
        ri.setUniform("lights[" + std::to_string(id) + "].position", vec3(lightSpaceMatrix[0][3], lightSpaceMatrix[1][3], lightSpaceMatrix[2][3]));
        // ri.setUniform("lights[" + std::to_string(id) + "].color", lightData.color);

        ri.changeUsedProgram(SHADOW_DEPTH);
        ri.setUniform("lightSpaceMatrix", lightSpaceMatrix);
        sceneRoot->lightPassDraw();
        ri.endCanva();

        ri.changeUsedProgram(PBR);
        ri.bindTexture(lightData.shadowMapID, "shadowMap");
        ri.setUniform("lightSpaceMatrix", lightSpaceMatrix);
        currentLightIdx++;
    }

    //     beginCanva(transform.getGlobalModelMatrix(), dim, m_shadowMapFrameBufferID, FramebufferTextureFormat::DEPTH24);
    // attachTextureToDepthFramebuffer(lightTextureID, m_shadowMapFrameBufferID);
    // Application::getInstance().getRootNodePtr()->lightPassDraw();
    // m_frontend.changeUsedProgram(ProgramType::TEXTURE);
    // m_frontend.bindTexture(lightTextureID, "sampledTexture");
    // m_frontend.submit(m_debugPlane, transfo);
    // m_frontend.endCanva();
}

renderID LightManager::getProbesRadianceTexture()
{
    return m_probeRadianceTexture;
}

void LightManager::updateProbeField()
{
    auto& ri = Renderer::getInstance();
    mat4 identity(1);

    ri.beginCanva(identity, identity, m_probesFrameBuffer, FramebufferTextureFormat::DEPTH24RGBA8);
    ri.renderFromPoint(vec3(0), *Application::getInstance().getRootNodePtr().get(), m_renderingCubemap);

    ri.changeUsedProgram(ProgramType::COMPUTE_OCTAHEDRAL);
    ri.useCubemap(m_renderingCubemap, "environmentMap");
    ri.setUniform("scale", 1.f);
    ri.setViewport(vec2(0), vec2(1024));
    ri.submit(m_fullQuad);

    ri.endCanva();
}

} // namespace Galaxy
