#include "LightManager.hpp"
#include "core/Application.hpp"
#include "engine/rendering/CameraManager.hpp"
#include "engine/rendering/renderer/Renderer.hpp"

namespace Galaxy {
LightManager::LightManager()
    : m_gridDimX(0)
    , m_gridDimY(0)
    , m_gridDimZ(0)
    , m_probeDistance(1.f)
    , m_textureWidth(2048)
    , m_textureHeight(1024)
    , m_probeResolution(512)
    , m_dirty(true)
    , m_maxLightCount(32)
{
    for(size_t i=0; i<m_maxLightCount; i++){
        m_availableIDs.emplace(static_cast<lightID>(i));
    }
}

LightManager::~LightManager()
{
}

void LightManager::init()
{
    auto& backend = Renderer::getInstance().getBackend();
    // m_shadowMapFramebuffer = backend.instanciateFrameBuffer(1024, 1024, FramebufferTextureFormat::DEPTH, 0, maxLightCount);
    
    // m_fullQuad = backend.generateQuad(vec2(2, 2), []() {});
    
    // // frontend.resizeCubemap(m_colorRenderingCubemap, m_probeResolution);
    
    // // TODO: pass to a format for normals in addition to colors and depths
    // m_probesFramebuffer = backend.instanciateFrameBuffer(m_textureWidth, m_textureHeight, FramebufferTextureFormat::DEPTH24RGBA8, 4);
    // m_cubemapFramebuffer = backend.instantiateCubemapFrameBuffer(1024, 3);

    // m_debugStartGeometry = backend.generateCube(1.f, false, []() {});
    // m_debugEndGeometry   = backend.generateCube(1.f, false, []() {});

    // m_debugStartTransform.translate(vec3(-50.f, 10.f, -80.f));
    // m_debugEndTransform.translate(vec3(80.f, 50.f, 80.f));
    // m_debugStartTransform.computeModelMatrix();
    // m_debugEndTransform.computeModelMatrix();

    // resizeProbeFieldGrid(2, 2, 2, 100.f);

    m_lightsUBO = backend.instantiateUBO(sizeof(m_lightUniformData));

    // auto& frontend = Renderer::getInstance().getFrontend();
    // frontend.bindUBO(m_lightsUBO, 0);
}

int LightManager::registerLight(LightData desc)
{
    if(!m_availableIDs.empty()){
        lightID id              = m_availableIDs.top();
        desc.idx                = static_cast<int>(id);
        m_availableIDs.pop();
        m_lights[id]            = desc;
    
        m_lights[id].shadowMapLayer = id;
        m_dirty = true;
    
        return id;
    } else {
        GLX_CORE_WARN("Max light count ({0}) reached", m_maxLightCount);
        // GLX-TODO: how to return error ?
        return -1;
    }
}

void LightManager::updateLightTransform(lightID id, math::mat4 transform)
{
    m_lights[id].transformationMatrix = transform;
    m_dirty = true;
}

void LightManager::updateLightColor(lightID id, math::vec3 color)
{
    m_lights[id].color      = color;
    m_dirty = true;
}

void LightManager::updateLightIntensity(lightID id, float intensity)
{
    m_lights[id].intensity  = intensity;
    m_dirty = true;
}

void LightManager::updateLightRange(lightID id, float range)
{
    m_lights[id].range      = range;
    m_dirty = true;
}

void LightManager::updateLightCutoffs(lightID id, float innerCutoff, float outerCutoff)
{
    m_lights[id].innerCutoff      = innerCutoff;
    m_lights[id].outerCutoff      = outerCutoff;
    m_dirty = true;
}

void LightManager::updateLightCastShadow(lightID id, bool state)
{
    m_lights[id].castShadow = state;
    m_dirty = true;
}

void LightManager::unregisterLight(int id)
{
    m_lights.erase(id);
    m_availableIDs.emplace(id);
    m_dirty = true;
}


void LightManager::debugDraw()
{
    // auto& frontend = Renderer::getInstance().getFrontend();

    // frontend.submit(m_debugStartGeometry, m_debugStartTransform);
    // frontend.submit(m_debugEndGeometry, m_debugEndTransform);

    // vec3 debugStart = m_debugStartTransform.getGlobalPosition();
    // vec3 debugEnd   = m_debugEndTransform.getGlobalPosition();

    // frontend.changeUsedProgram(POST_PROCESSING_PROBE);
    // frontend.setFramebufferAsTextureUniform(m_probesFramebuffer, "probeIrradianceField", 0);
    // // frontend.setFramebufferAsTextureUniform(m_probesFramebuffer, "probeColorField", 1);
    // frontend.setFramebufferAsTextureUniform(m_probesFramebuffer, "probeNormalField", 2);
    // frontend.setFramebufferAsTextureUniform(m_probesFramebuffer, "probeDepthField", 3);
    // // frontend.bindTexture(m_probeRadianceTexture, "probeIrradianceField");
    // // frontend.bindTexture(m_probeDepthTexture, "probeDepthField");

    // // frontend.submitDebugLine(debugStart, debugEnd);
}

std::vector<vec3> LightManager::getProbePositions()
{
    std::vector<vec3> res(m_probeGrid.size());
    for (int i = 0; i < res.size(); i++) {
        res[i] = m_probeGrid[i].position;
    }
    return res;
}

UpdateUBOCommand LightManager::getLightUboUpdate()
{
    vec2 viewportDimmension = vec2(1024, 1024);
    for (auto& light : m_lights) {
        auto& lightData = light.second;
        mat4 projMat = lightData.getProjection(viewportDimmension);

        m_lightUniformData.colors[lightData.idx]    = vec4(lightData.color, 1.0);
        m_lightUniformData.positions[lightData.idx] = lightData.transformationMatrix[3];
        const vec3 direction = normalize(-vec3(lightData.transformationMatrix[2]));
        m_lightUniformData.directions[lightData.idx] = vec4(direction, 0.0f);
        m_lightUniformData.params[lightData.idx].x  = lightData.intensity;
        m_lightUniformData.params[lightData.idx].y  = lightData.range;
        m_lightUniformData.params[lightData.idx].z  = lightData.innerCutoff;
        m_lightUniformData.params[lightData.idx].w  = lightData.outerCutoff;
        m_lightUniformData.shadowMapLayers[lightData.idx].layer = lightData.shadowMapLayer;
        m_lightUniformData.shadowMapLayers[lightData.idx].type = lightData.type;
        m_lightUniformData.shadowMapLayers[lightData.idx].castShadow = lightData.castShadow ? 1 : 0;


        mat4 view             = CameraManager::processViewMatrix(lightData.transformationMatrix);
        mat4 lightSpaceMatrix = projMat * view;
        m_lightUniformData.lightMatrices[lightData.idx] = lightSpaceMatrix;
    }

    return UpdateUBOCommand::make(m_lightsUBO, m_lightUniformData);
}

const std::vector<LightData> LightManager::getLightsData() const
{
    std::vector<LightData> res;
    res.reserve(m_lights.size());

    vec2 viewportDimmension = vec2(1024, 1024);
    for (auto& [id, lightData] : m_lights) {
        res.push_back(lightData);
    }

    return res;
}

void LightManager::updateProbeField()
{
    // auto& frontend = Renderer::getInstance().getFrontend();
    // mat4 identity(1);

    // vec3 debugStart = m_debugStartTransform.getGlobalPosition();
    // vec3 debugEnd   = m_debugEndTransform.getGlobalPosition();

    // auto initDevice = std::make_unique<RenderDevice>();
    // initDevice->renderScene = false;
    // initDevice->noClear = false;
    // initDevice->targetFramebuffer = m_probesFramebuffer;
    // frontend.addRenderDevice(std::move(initDevice));
    // frontend.changeUsedProgram(ProgramType::COMPUTE_OCTAHEDRAL);
    // frontend.setFramebufferAsCubemapUniform(m_cubemapFramebuffer, "radianceCubemap", 0);
    // frontend.setFramebufferAsCubemapUniform(m_cubemapFramebuffer, "normalCubemap", 1);
    // frontend.setFramebufferAsCubemapUniform(m_cubemapFramebuffer, "depthCubemap", -1);
    // frontend.changeUsedProgram(ProgramType::PBR);
    // frontend.setUniform("includeLightComputation", false);
    
    
    // for (auto& probe : m_probeGrid) {
    //     auto renderPoint = std::make_unique<RenderPoint>();
    //     renderPoint->targetCubemapFramebuffer = m_cubemapFramebuffer;
        
    //     Transform renderTransform;
    //     renderTransform.setLocalPosition(probe.position);
    //     renderTransform.computeModelMatrix();

    //     std::shared_ptr<Camera> pointCam = std::make_shared<Camera>();
    //     pointCam->dimmensions = vec2(1024);
    //     pointCam->position = renderTransform.getGlobalPosition();
    //     renderPoint->camera = pointCam;
    //     renderPoint->frustumCulling = false;

    //     renderPoint->renderScene = true;
        
    //     frontend.addRenderDevice(std::move(renderPoint));
        
    //     // frontend.setUniform("scale", vec2(m_textureWidth / (float)m_probeResolution, m_textureHeight / (float)m_probeResolution));
        
        
    //     auto octahedralProjectionDevice = std::make_unique<RenderDevice>();
    //     octahedralProjectionDevice->targetFramebuffer = m_probesFramebuffer;
    //     octahedralProjectionDevice->noClear = true;
    //     octahedralProjectionDevice->renderScene = false;
        
    //     octahedralProjectionDevice->viewportDimmension = vec2(m_probeResolution);

    //     octahedralProjectionDevice->viewportPosition = getProbeTexCoord(probe.probeCoord);
    //     frontend.addRenderDevice(std::move(octahedralProjectionDevice));
        
    //     frontend.changeUsedProgram(ProgramType::COMPUTE_OCTAHEDRAL);
    //     frontend.submit(m_fullQuad);
    // }
    
    // frontend.changeUsedProgram(ProgramType::PBR);
    // frontend.setUniform("includeLightComputation", true);

    // // frontend.beginCanva(identity, identity, m_probesFrameBuffer, FramebufferTextureFormat::DEPTH24RGBA8);
    // // frontend.avoidCanvaClear();
    // // frontend.saveCanvaResult("probes");
    // // frontend.endCanva();
}

void LightManager::updateBias(float newValue)
{
    // Renderer::getInstance().getFrontend().changeUsedProgram(ProgramType::POST_PROCESSING_PROBE);
    // Renderer::getInstance().getFrontend().setUniform("traceBias", newValue);
}

void LightManager::resizeProbeFieldGrid(unsigned int width, unsigned int height, unsigned int depth, float spaceBetween, unsigned int probeTextureResolution, vec3 probeFieldCenter)
{
    // m_gridDimX        = width;
    // m_gridDimY        = height;
    // m_gridDimZ        = depth;
    // m_probeDistance   = spaceBetween;
    // m_probeResolution = probeTextureResolution;
    // m_probeFieldStart = probeFieldCenter - vec3(m_gridDimX - 1, m_gridDimY - 1, m_gridDimZ - 1) * spaceBetween / 2.0;

    // m_probeGrid.resize(width * height * depth);

    // m_textureWidth  = width * height * m_probeResolution;
    // m_textureHeight = depth * m_probeResolution;

    // Renderer::getInstance().getBackend().resizeFrameBuffer(m_probesFramebuffer, m_textureWidth, m_textureHeight);

    // for (int z = 0; z < depth; z++) {
    //     for (int y = 0; y < height; y++) {
    //         for (int x = 0; x < width; x++) {
    //             vec3 position(x, y, z);
    //             position *= m_probeDistance;
    //             position += m_probeFieldStart;

    //             unsigned int probeIdx            = getCellCoord(x, y, z);
    //             m_probeGrid[probeIdx].probeCoord = probeIdx;
    //             m_probeGrid[probeIdx].position   = position;
    //         }
    //     }
    // }

    // auto& frontend = Renderer::getInstance().getFrontend();
    // frontend.changeUsedProgram(ProgramType::POST_PROCESSING_PROBE);
    // frontend.setUniform("probeFieldGridDim", ivec3(m_gridDimX, m_gridDimY, m_gridDimZ));
    // frontend.setUniform("probeFieldCellSize", m_probeDistance);
    // frontend.setUniform("probeTextureSingleSize", (int)m_probeResolution);
    // frontend.setUniform("probeFieldOrigin", m_probeFieldStart);
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

mat4 LightData::getProjection(const vec2& viewportDimmension)
{
    return CameraManager::processProjectionMatrix(viewportDimmension, 0.01f, range, outerCutoff);
}

} // namespace Galaxy
