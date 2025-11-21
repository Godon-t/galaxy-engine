#include "LightManager.hpp"
#include "engine/rendering/CameraManager.hpp"
#include "engine/rendering/renderer/Renderer.hpp"

namespace Galaxy {
LightManager::LightManager()
{
}

LightManager::~LightManager()
{
}

void LightManager::init()
{
    m_shadowMapFrameBufferID = Renderer::getInstance().instanciateFrameBuffer(1024, 1024, FramebufferTextureFormat::DEPTH);
}

int LightManager::registerLight(const SpotLight* desc)
{
    lightID id = m_nextLightID++;
    int idx    = m_currentLightCount++;

    const mat4& lightSpaceMatrix = desc->getTransform()->getGlobalModelMatrix(); // Use
    mat4 lightViewMatrix         = CameraManager::processViewMatrix(lightSpaceMatrix);
    m_lights[id]                 = LightData(idx, lightViewMatrix);

    renderID shadowMapID     = Renderer::getInstance().instantiateTexture();
    m_lights[id].shadowMapID = shadowMapID;
    return id;
}

void LightManager::updateLightTransform(lightID id, math::mat4 transform)
{
    m_lights[id].lightSpaceMatrix = transform;
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

        ri.beginCanva(lightData.lightSpaceMatrix, projMat, m_shadowMapFrameBufferID, FramebufferTextureFormat::DEPTH24STENCIL8);
        ri.setUniform("lightSpaceMatrix[" + std::to_string(id) + "]", m_lights[id].lightSpaceMatrix);
        ri.linkCanvaDepthToTexture(lightData.shadowMapID);
        // ri.attachTextureToDepthFramebuffer(lightData.shadowMapID, m_shadowMapFrameBufferID);
        sceneRoot->lightPassDraw();
        ri.endCanva();
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

} // namespace Galaxy
