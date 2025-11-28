#include "LightManager.hpp"
#include "core/Application.hpp"
#include "engine/rendering/CameraManager.hpp"
#include "engine/rendering/renderer/Renderer.hpp"

namespace Galaxy {
LightManager::LightManager()
    : m_shadowMapFrameBufferID(0)
    , m_probesFrameBuffer(0)
    , m_colorRenderingCubemap(0)
    , m_depthRenderingCubemap(0)
    , m_fullQuad(0)
    , m_gridDimX(0)
    , m_gridDimY(0)
    , m_gridDimZ(0)
    , m_probeDistance(1.f)
    , m_textureWidth(2048)
    , m_textureHeight(1024)
    , m_probeResolution(512)
{
}

LightManager::~LightManager()
{
}

renderID startVisu(0);
renderID endVisu(0);

void LightManager::init()
{
    auto& ri                 = Renderer::getInstance();
    m_shadowMapFrameBufferID = ri.instanciateFrameBuffer(1024, 1024, FramebufferTextureFormat::DEPTH);

    m_fullQuad = ri.generateQuad(vec2(2, 2), []() {});

    m_colorRenderingCubemap = ri.instanciateCubemap();
    m_depthRenderingCubemap = ri.instanciateCubemap();
    // ri.resizeCubemap(m_colorRenderingCubemap, m_probeResolution);

    // TODO: pass to a format for normals in addition to colors and depths
    m_probesFrameBuffer = ri.instanciateFrameBuffer(m_textureWidth, m_textureHeight, FramebufferTextureFormat::DEPTH24RGBA8);
    ri.beginCanvaNoBuffer();
    // ri.attachTextureToDepthFramebuffer(m_probeDepthTexture, m_probesFrameBuffer);
    // ri.attachTextureToColorFramebuffer(m_probeRadianceTexture, m_probesFrameBuffer);

    m_debugStartVisu = ri.generateCube(0.2f, false, []() {});
    m_debugEndVisu   = ri.generateCube(0.2f, false, []() {});

    m_debugStartTransform.translate(vec3(-500.f, 10.f, -80.f));
    m_debugEndTransform.translate(vec3(800.f, 50.f, 80.f));
    m_debugStartTransform.computeModelMatrix();
    m_debugEndTransform.computeModelMatrix();

    resizeProbeFieldGrid(2, 2, 2, 100.f);

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

void LightManager::debugDraw()
{
    auto& ri = Renderer::getInstance();

    ri.submit(m_debugStartVisu, m_debugStartTransform);
    ri.submit(m_debugEndVisu, m_debugEndTransform);
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

    vec3 debugStart = m_debugStartTransform.getGlobalPosition();
    vec3 debugEnd  = m_debugEndTransform.getGlobalPosition();

    for (auto& probe : m_probeGrid) {
        ri.renderFromPoint(probe.position, *Application::getInstance().getRootNodePtr().get(), m_colorRenderingCubemap, m_depthRenderingCubemap);

        ri.beginCanva(identity, identity, m_probesFrameBuffer, FramebufferTextureFormat::DEPTH24RGBA8);
        ri.avoidCanvaClear();
        ri.changeUsedProgram(ProgramType::COMPUTE_OCTAHEDRAL);
        // ri.setUniform("scale", vec2(m_textureWidth / (float)m_probeResolution, m_textureHeight / (float)m_probeResolution));
        ri.useCubemap(m_colorRenderingCubemap, "radianceCubemap");
        ri.useCubemap(m_depthRenderingCubemap, "depthCubemap");


        ri.setUniform("debugStart", debugStart);
        ri.setUniform("debugEnd", debugEnd);
        ri.setUniform("debugProbePos", probe.position);


        vec2 viewportCoords = getProbeTexCoord(probe.probeCoord);
        ri.setViewport(viewportCoords, vec2(m_probeResolution));
        ri.changeUsedProgram(ProgramType::COMPUTE_OCTAHEDRAL);
        ri.submit(m_fullQuad);
        ri.endCanva();
    }

    ri.beginCanva(identity, identity, m_probesFrameBuffer, FramebufferTextureFormat::DEPTH24RGBA8);
    ri.avoidCanvaClear();
    ri.saveCanvaResult("probes");
    ri.endCanva();
}

void LightManager::resizeProbeFieldGrid(unsigned int width, unsigned int height, unsigned int depth, float spaceBetween)
{
    m_gridDimX      = width;
    m_gridDimY      = height;
    m_gridDimZ      = depth;
    m_probeDistance = spaceBetween;

    m_probeGrid.resize(width * height * depth);

    m_textureWidth  = width * height * m_probeResolution;
    m_textureHeight = depth * m_probeResolution;

    Renderer::getInstance().resizeFrameBuffer(m_probesFrameBuffer, m_textureWidth, m_textureHeight);

    for (int z = 0; z < depth; z++) {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                vec3 position(x, y, z);
                position *= m_probeDistance;

                unsigned int probeIdx            = getCellCoord(x, y, z);
                m_probeGrid[probeIdx].probeCoord = probeIdx;
                m_probeGrid[probeIdx].position   = position;
            }
        }
    }
}

unsigned int LightManager::getCellCoord(unsigned int x, unsigned int y, unsigned int z)
{
    return z * m_gridDimX * m_gridDimY + y * m_gridDimX + x;
}

vec2 LightManager::getProbeTexCoord(unsigned int probeGridIdx)
{
    unsigned int probesByWidth = m_textureWidth / m_probeResolution;
    unsigned int xPosition     = (probeGridIdx % probesByWidth) * m_probeResolution;
    unsigned int yPosition     = (probeGridIdx / probesByWidth) * m_probeResolution;
    vec2 texturePos(xPosition, yPosition);
    return texturePos;
}

} // namespace Galaxy
