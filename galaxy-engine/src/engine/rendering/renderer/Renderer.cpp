#include "Renderer.hpp"

#include "pch.hpp"

#include "Application.hpp"
#include "Core.hpp"
#include "gl_headers.hpp"
#include "rendering/CameraManager.hpp"
#include "rendering/GPUInstances/FrameBuffer.hpp"
#include "rendering/OpenglHelper.hpp"

namespace Galaxy {
Renderer::Renderer()
    : m_commandBuffers(2)
    , m_frontCommandBufferIdx(0)
    , m_frontend(&m_commandBuffers[m_frontCommandBufferIdx])
    , m_backend()
    , m_lightManager()
    , m_mainViewportSize(1024)
    , m_activePostProcessing(ProgramType::POST_PROCESSING_PROBE)
    , m_showDebug(false)
{
    m_backend.initDebugCallback();

    m_cubemapFramebufferID   = m_backend.instantiateCubemapFrameBuffer(1024);
    m_sceneFrameBufferID     = m_backend.instanciateFrameBuffer(100, 100, FramebufferTextureFormat::DEPTH24RGBA8, 2);
    m_postProcessingBufferID = m_backend.instanciateFrameBuffer(100, 100, FramebufferTextureFormat::RGBA8);
    m_postProcessingQuadID   = m_backend.generateQuad(vec2(2, 2), [] {});

    m_cubemap_orientations[0] = { 1, 0, 0 };
    m_cubemap_orientations[1] = { -1, 0, 0 };
    m_cubemap_ups[0] = m_cubemap_ups[1] = { 0, -1, 0 };

    m_cubemap_orientations[2] = { 0, 1, 0 };
    m_cubemap_orientations[3] = { 0, -1, 0 };
    m_cubemap_ups[2]          = { 0, 0, 1 };
    m_cubemap_ups[3]          = { 0, 0, -1 };

    m_cubemap_orientations[4] = { 0, 0, 1 };
    m_cubemap_orientations[5] = { 0, 0, -1 };
    m_cubemap_ups[4] = m_cubemap_ups[5] = { 0, -1, 0 };

    // m_frontend.attachTextureToDepthFramebuffer(m_testRectText, m_testRectFB);
}

Renderer::~Renderer()
{
    m_backend.destroy();
}

void Renderer::switchCommandBuffer()
{
    m_frontCommandBufferIdx = 1 - m_frontCommandBufferIdx;
    m_frontend.setCommandBuffer(&m_commandBuffers[m_frontCommandBufferIdx]);
}

Renderer& Renderer::getInstance()
{
    static Renderer renderer;
    return renderer;
}

void Renderer::init()
{
    m_lightManager.init();
}

void Renderer::beginSceneRender(const mat4& camTransform)
{
    beginCanva(camTransform, m_mainViewportSize, m_sceneFrameBufferID, FramebufferTextureFormat::DEPTH24RGBA8);
    m_frontend.setViewport(vec2(0), m_mainViewportSize);
}

void Renderer::beginCanva(const mat4& camTransform, const vec2& dimmensions, renderID framebufferID, FramebufferTextureFormat framebufferFormat, int cubemapIdx)
{
    m_currentView = CameraManager::processViewMatrix(camTransform);
    m_currentProj = CameraManager::processProjectionMatrix(dimmensions);

    m_frontend.beginCanva(m_currentView, m_currentProj, framebufferID, framebufferFormat);
}

void Renderer::endSceneRender()
{
    m_lightManager.debugDraw();
    m_frontend.drawDebug();
    m_frontend.endCanva();
    applyPostProcessing();
    m_frontend.processCanvas();
}

void Renderer::shadowPass()
{
    m_lightManager.shadowPass(Application::getInstance().getRootNodePtr().get());
}

void Renderer::applyPostProcessing()
{
    if (m_activePostProcessing != ProgramType::POST_PROCESSING_PROBE && m_activePostProcessing != ProgramType::POST_PROCESSING_SSGI)
        GLX_CORE_ASSERT(m_activePostProcessing == ProgramType::POST_PROCESSING_PROBE || m_activePostProcessing == ProgramType::POST_PROCESSING_SSGI, "Wrong porgram type for post processing");

    m_frontend.beginCanva(m_currentView, m_currentProj, m_postProcessingBufferID, FramebufferTextureFormat::RGBA8);
    m_frontend.changeUsedProgram(m_activePostProcessing);

    m_frontend.initPostProcessing(m_sceneFrameBufferID);
    m_frontend.submit(m_postProcessingQuadID);
    m_frontend.changeUsedProgram(ProgramType::PBR);
    m_frontend.endCanva();
}

void Renderer::renderFrame()
{
    m_drawCount = m_commandBuffers[m_frontCommandBufferIdx].size();
    m_backend.processCommands(m_commandBuffers[m_frontCommandBufferIdx]);
    m_commandBuffers[m_frontCommandBufferIdx].clear();
    switchCommandBuffer();
}

renderID Renderer::instanciateMaterial(ResourceHandle<Material> material)
{
    renderID matID = m_backend.instanciateMaterial(material);

    auto& matResource = material.getResource();
    matResource.onLoaded([this, matID, &matResource] {
        m_frontend.setTransparency(matID, matResource.isUsingTransparency());
    });

    return matID;
}

void Renderer::updateMaterial(renderID materialID, ResourceHandle<Material> material)
{
    m_frontend.setTransparency(materialID, material.getResource().isUsingTransparency());
}

void Renderer::applyFilterOnCubemap(renderID skyboxMesh, renderID sourceID, renderID targetID, FilterEnum filter)
{
    // switchCommandBuffer();
    // m_commandBuffers[m_frontCommandBufferIdx].clear();

    // std::function<void()> prgToUse;
    // switch (filter) {
    // case FilterEnum::IRRADIANCE:
    //     prgToUse = [this]() {
    //         m_frontend.changeUsedProgram(ProgramType::FILTER_IRRADIANCE);
    //     };
    //     break;
    // default:
    //     GLX_CORE_ERROR("Unknown filter applied!");
    //     return;
    // }

    // Cubemap& targetCubemap = *m_backend.m_cubemapInstances.get(targetID);
    // targetCubemap.useFloat = true;
    // targetCubemap.resize(2048);
    // CubemapFrameBuffer cubemapBuffer(targetCubemap);

    // GLint viewport[4];
    // glGetIntegerv(GL_VIEWPORT, viewport);
    // glViewport(0, 0, targetCubemap.resolution, targetCubemap.resolution);

    // vec2 dimmensions      = vec2(targetCubemap.resolution);
    // auto projectionMatrix = CameraManager::processProjectionMatrix(vec2(dimmensions));

    // mat4 baseProjection = m_frontend.getProjectionMatrix();
    // mat4 projection     = perspective(radians(90.0f), 1.f, 0.001f, 999.f);
    // m_frontend.setProjectionMatrix(projection);

    // vec3 position = vec3(0, 0, 0);
    // Transform transformTemp;
    // for (int i = 0; i < 6; i++) {
    //     auto viewMatrix = lookAt(camPosition, camPosition + camDirection, camUp);
    //     m_frontend.beginCanva(viewMatrix, projectionMatrix, targetID, FramebufferTextureFormat::DEPTH24RGBA8, i);

    //     beginSceneRender(position, m_cubemap_orientations[i], m_cubemap_ups[i], dimmensions);
    //     prgToUse();
    //     m_frontend.bindCubemap(sourceID, "skybox");
    //     m_frontend.submit(skyboxMesh, transformTemp);
    //     endSceneRender();
    //     renderFrame();
    // }

    // m_frontend.setProjectionMatrix(baseProjection);
    // glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

void Renderer::renderFromPoint(vec3 position, Node& root, renderID targetColorCubemapID, renderID targetNormalCubemapID, renderID targetDepthCubemapID)
{
    vec2 size(1024);
    vec2 pos(0);

    m_frontend.beginCanvaNoBuffer();
    m_frontend.attachCubemapToFramebuffer(targetColorCubemapID, m_cubemapFramebufferID, 0);
    m_frontend.attachCubemapToFramebuffer(targetNormalCubemapID, m_cubemapFramebufferID, 1);
    m_frontend.attachCubemapToFramebuffer(targetDepthCubemapID, m_cubemapFramebufferID, 2);
    m_frontend.endCanva();

    mat4 projection = perspective(radians(90.0f), 1.f, 0.001f, 999.f);
    for (int i = 0; i < 6; i++) {
        auto viewMatrix = lookAt(position, position + m_cubemap_orientations[i], m_cubemap_ups[i]);

        m_frontend.beginCanva(viewMatrix, projection, m_cubemapFramebufferID, FramebufferTextureFormat::RGBA8, i);
        m_frontend.setViewport(pos, size);
        root.draw();
        m_lightManager.debugDraw();
        m_frontend.drawDebug();
        m_frontend.endCanva();
    }
}

}
