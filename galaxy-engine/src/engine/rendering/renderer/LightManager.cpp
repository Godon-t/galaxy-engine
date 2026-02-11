#include "LightManager.hpp"
#include "core/Application.hpp"
#include "engine/rendering/CameraManager.hpp"
#include "engine/rendering/renderer/Renderer.hpp"

namespace Galaxy {
LightManager::LightManager()
    : m_shadowMapFrameBufferID(0)
    , m_probesFrameBuffer(0)
    , m_fullQuad(0)
    , m_gridDimX(0)
    , m_gridDimY(0)
    , m_gridDimZ(0)
    , m_probeDistance(1.f)
    , m_textureWidth(2048)
    , m_textureHeight(1024)
    , m_probeResolution(512)
    , m_lightsUBO(0)
{
}

LightManager::~LightManager()
{
}

renderID startVisu(0);
renderID endVisu(0);

void LightManager::init()
{
    auto& backend = Renderer::getInstance().getBackend();
    m_shadowMapFrameBufferID = backend.instanciateFrameBuffer(1024, 1024, FramebufferTextureFormat::DEPTH);
    
    m_fullQuad = backend.generateQuad(vec2(2, 2), []() {});
    
    // frontend.resizeCubemap(m_colorRenderingCubemap, m_probeResolution);
    
    // TODO: pass to a format for normals in addition to colors and depths
    m_probesFrameBuffer = backend.instanciateFrameBuffer(m_textureWidth, m_textureHeight, FramebufferTextureFormat::DEPTH24RGBA8, 4);
    m_cubemapFramebufferID   = backend.instantiateCubemapFrameBuffer(1024, 3);

    m_debugStartVisu = backend.generateCube(1.f, false, []() {});
    m_debugEndVisu   = backend.generateCube(1.f, false, []() {});

    m_debugStartTransform.translate(vec3(-50.f, 10.f, -80.f));
    m_debugEndTransform.translate(vec3(80.f, 50.f, 80.f));
    m_debugStartTransform.computeModelMatrix();
    m_debugEndTransform.computeModelMatrix();

    resizeProbeFieldGrid(2, 2, 2, 100.f);

    m_lightsUBO = backend.instantiateUBO(sizeof(m_lightUniformData));

    auto& frontend = Renderer::getInstance().getFrontend();
    frontend.bindUBO(m_lightsUBO, 0);
}

int LightManager::registerLight(LightData desc)
{
    desc.idx                = m_currentLightCount++;
    lightID id              = m_nextLightID++;
    m_lights[id]            = desc;
    m_lights[id].needUpdate = true;

    auto& frontend = Renderer::getInstance().getFrontend();
    frontend.changeUsedProgram(ProgramType::PBR);
    frontend.setUniform("lightCount", m_currentLightCount);

    // renderID shadowMapID     = Renderer::getInstance().instantiateTexture();
    // m_lights[id].shadowMapID = shadowMapID;

    return id;
}

void LightManager::updateLightTransform(lightID id, math::mat4 transform)
{
    m_lights[id].transformationMatrix = transform;
    m_lights[id].needUpdate           = true;
}

void LightManager::updateLightColor(lightID id, math::vec3 color)
{
    m_lights[id].color      = color;
    m_lights[id].needUpdate = true;
}

void LightManager::updateLightIntensity(lightID id, float intensity)
{
    m_lights[id].intensity  = intensity;
    m_lights[id].needUpdate = true;
}

void LightManager::updateLightRange(lightID id, float range)
{
    m_lights[id].range      = range;
    m_lights[id].needUpdate = true;
}

void LightManager::unregisterLight(int id)
{
    m_lights.erase(id);
}

void LightManager::debugDraw()
{
    auto& frontend = Renderer::getInstance().getFrontend();

    frontend.submit(m_debugStartVisu, m_debugStartTransform);
    frontend.submit(m_debugEndVisu, m_debugEndTransform);

    vec3 debugStart = m_debugStartTransform.getGlobalPosition();
    vec3 debugEnd   = m_debugEndTransform.getGlobalPosition();

    frontend.changeUsedProgram(POST_PROCESSING_PROBE);
    frontend.setFramebufferAsTextureUniform(m_probesFrameBuffer, "probeIrradianceField", 0);
    frontend.setFramebufferAsTextureUniform(m_probesFrameBuffer, "probeColorField", 1);
    frontend.setFramebufferAsTextureUniform(m_probesFrameBuffer, "probeNormalField", 2);
    frontend.setFramebufferAsTextureUniform(m_probesFrameBuffer, "probeDepthField", 3);
    // frontend.bindTexture(m_probeRadianceTexture, "probeIrradianceField");
    // frontend.bindTexture(m_probeDepthTexture, "probeDepthField");

    // frontend.submitDebugLine(debugStart, debugEnd);
}

std::vector<vec3> LightManager::getProbePositions()
{
    std::vector<vec3> res(m_probeGrid.size());
    for (int i = 0; i < res.size(); i++) {
        res[i] = m_probeGrid[i].position;
    }
    return res;
}

void LightManager::shadowPass(Node* sceneRoot)
{
    auto& frontend = Renderer::getInstance().getFrontend();

    frontend.changeUsedProgram(PBR);
    bool updateUniform = false;
    for (auto& light : m_lights) {
        if (!light.second.needUpdate)
            continue;

        updateUniform           = true;
        light.second.needUpdate = false;

        m_lightUniformData.colors[light.second.idx]    = vec4(light.second.color, 1.0);
        m_lightUniformData.positions[light.second.idx] = light.second.transformationMatrix[3];
        m_lightUniformData.params[light.second.idx].y  = light.second.intensity;
        m_lightUniformData.params[light.second.idx].z  = light.second.range;

        // frontend.setUniform("lights[" + std::to_string(id) + "].lightMatrix", lightSpaceMatrix);
        // frontend.setUniform("lights[" + std::to_string(id) + "].position", vec3(lightSpaceMatrix[3]));
        // frontend.setUniform("lights[" + std::to_string(id) + "].color", lightData.color);
    }
    frontend.updateUniform(m_lightsUBO, m_lightUniformData);

    // math::mat4 projMat  = CameraManager::processProjectionMatrix(vec2(1024, 1024));
    // int currentLightIdx = 0;
    // for (auto& [id, lightData] : m_lights) {
    //     if (currentLightIdx >= m_maxLights)
    //         break;

    //     math::mat4 view             = CameraManager::processViewMatrix(lightData.transformationMatrix);
    //     math::mat4 lightSpaceMatrix = projMat * view;

    //     frontend.beginCanva(view, projMat, m_shadowMapFrameBufferID, FramebufferTextureFormat::DEPTH);
    //     frontend.linkCanvaDepthToTexture(lightData.shadowMapID);

    //     frontend.changeUsedProgram(SHADOW_DEPTH);
    //     frontend.setUniform("lightSpaceMatrix", lightSpaceMatrix);
    //     sceneRoot->lightPassDraw();
    //     frontend.endCanva();

    //     frontend.changeUsedProgram(PBR);
    //     frontend.bindTexture(lightData.shadowMapID, "shadowMap");
    //     frontend.setUniform("lightSpaceMatrix", lightSpaceMatrix);
    //     currentLightIdx++;
    // }

    //     beginCanva(transform.getGlobalModelMatrix(), dim, m_shadowMapFrameBufferID, FramebufferTextureFormat::DEPTH24);
    // attachTextureToDepthFramebuffer(lightTextureID, m_shadowMapFrameBufferID);
    // Application::getInstance().getRootNodePtr()->lightPassDraw();
    // m_frontend.changeUsedProgram(ProgramType::TEXTURE);
    // m_frontend.bindTexture(lightTextureID, "sampledTexture");
    // m_frontend.submit(m_debugPlane, transfo);
    // m_frontend.endCanva();
}

void LightManager::updateProbeField()
{
    auto& frontend = Renderer::getInstance().getFrontend();
    mat4 identity(1);

    vec3 debugStart = m_debugStartTransform.getGlobalPosition();
    vec3 debugEnd   = m_debugEndTransform.getGlobalPosition();

    auto initDevice = std::make_unique<RenderDevice>();
    initDevice->renderScene = false;
    initDevice->noClear = false;
    initDevice->targetFramebuffer = m_probesFrameBuffer;
    frontend.addRenderDevice(std::move(initDevice));
    frontend.changeUsedProgram(ProgramType::COMPUTE_OCTAHEDRAL);
    frontend.setFramebufferAsCubemapUniform(m_cubemapFramebufferID, "radianceCubemap", 0);
    frontend.setFramebufferAsCubemapUniform(m_cubemapFramebufferID, "normalCubemap", 1);
    frontend.setFramebufferAsCubemapUniform(m_cubemapFramebufferID, "depthCubemap", -1);
    frontend.changeUsedProgram(ProgramType::PBR);
    frontend.setUniform("includeLightComputation", false);
    
    
    for (auto& probe : m_probeGrid) {
        auto renderPoint = std::make_unique<RenderPoint>();
        renderPoint->targetFramebuffer = m_cubemapFramebufferID;
        
        Transform renderTransform;
        renderTransform.setLocalPosition(probe.position);
        renderTransform.computeModelMatrix();
        renderPoint->transform = renderTransform.getGlobalModelMatrix();
        renderPoint->renderScene = true;
        
        renderPoint->viewportDimmmensions = vec2(1024);
        
        frontend.addRenderDevice(std::move(renderPoint));
        
        // frontend.setUniform("scale", vec2(m_textureWidth / (float)m_probeResolution, m_textureHeight / (float)m_probeResolution));
        
        
        auto octahedralProjectionDevice = std::make_unique<RenderDevice>();
        octahedralProjectionDevice->targetFramebuffer = m_probesFrameBuffer;
        octahedralProjectionDevice->noClear = true;
        octahedralProjectionDevice->renderScene = false;
        octahedralProjectionDevice->viewportDimmmensions = vec2(m_probeResolution);
        octahedralProjectionDevice->viewportPosition = getProbeTexCoord(probe.probeCoord);
        frontend.addRenderDevice(std::move(octahedralProjectionDevice));
        
        frontend.changeUsedProgram(ProgramType::COMPUTE_OCTAHEDRAL);
        frontend.submit(m_fullQuad);
    }
    
    frontend.changeUsedProgram(ProgramType::PBR);
    frontend.setUniform("includeLightComputation", true);

    // frontend.beginCanva(identity, identity, m_probesFrameBuffer, FramebufferTextureFormat::DEPTH24RGBA8);
    // frontend.avoidCanvaClear();
    // frontend.saveCanvaResult("probes");
    // frontend.endCanva();
}

void LightManager::updateBias(float newValue)
{
    Renderer::getInstance().getFrontend().changeUsedProgram(ProgramType::POST_PROCESSING_PROBE);
    Renderer::getInstance().getFrontend().setUniform("traceBias", newValue);
}

void LightManager::resizeProbeFieldGrid(unsigned int width, unsigned int height, unsigned int depth, float spaceBetween, unsigned int probeTextureResolution, vec3 probeFieldCenter)
{
    m_gridDimX        = width;
    m_gridDimY        = height;
    m_gridDimZ        = depth;
    m_probeDistance   = spaceBetween;
    m_probeResolution = probeTextureResolution;
    m_probeFieldStart = probeFieldCenter - vec3(m_gridDimX - 1, m_gridDimY - 1, m_gridDimZ - 1) * spaceBetween / 2.0;

    m_probeGrid.resize(width * height * depth);

    m_textureWidth  = width * height * m_probeResolution;
    m_textureHeight = depth * m_probeResolution;

    Renderer::getInstance().getBackend().resizeFrameBuffer(m_probesFrameBuffer, m_textureWidth, m_textureHeight);

    for (int z = 0; z < depth; z++) {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                vec3 position(x, y, z);
                position *= m_probeDistance;
                position += m_probeFieldStart;

                unsigned int probeIdx            = getCellCoord(x, y, z);
                m_probeGrid[probeIdx].probeCoord = probeIdx;
                m_probeGrid[probeIdx].position   = position;
            }
        }
    }

    auto& frontend = Renderer::getInstance().getFrontend();
    frontend.changeUsedProgram(ProgramType::POST_PROCESSING_PROBE);
    frontend.setUniform("probeFieldGridDim", ivec3(m_gridDimX, m_gridDimY, m_gridDimZ));
    frontend.setUniform("probeFieldCellSize", m_probeDistance);
    frontend.setUniform("probeTextureSingleSize", (int)m_probeResolution);
    frontend.setUniform("probeFieldOrigin", m_probeFieldStart);
}

unsigned int LightManager::getCellCoord(unsigned int x, unsigned int y, unsigned int z)
{
    return z * m_gridDimX * m_gridDimY + y * m_gridDimX + x;
}

vec2 LightManager::getProbeTexCoord(unsigned int probeGridIdx)
{
    unsigned int probesByWidth = m_gridDimX * m_gridDimY;
    unsigned int xPosition     = (probeGridIdx % probesByWidth) * m_probeResolution;
    unsigned int yPosition     = (probeGridIdx / probesByWidth) * m_probeResolution;
    vec2 texturePos(xPosition, yPosition);
    return texturePos;
}

} // namespace Galaxy
