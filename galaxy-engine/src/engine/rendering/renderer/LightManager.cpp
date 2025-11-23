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

} // namespace Galaxy
