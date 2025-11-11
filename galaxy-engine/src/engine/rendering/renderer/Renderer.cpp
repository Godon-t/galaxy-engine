#include "Renderer.hpp"

#include "pch.hpp"

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
{
    m_sceneFrameBufferID   = m_backend.instanciateFrameBuffer(1920, 1080, FramebufferTextureFormat::DEPTH24RGBA8);
    m_postProcessingQuadID = m_backend.generateQuad(vec2(2, 2), [] {});

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

void Renderer::beginSceneRender(const mat4& camTransform)
{
    m_frontend.bindFrameBuffer(m_sceneFrameBufferID, m_backend.getFramebufferFormat(m_sceneFrameBufferID));
    auto clearColor = math::vec4(0.2, 0.2, 0.25, 1.0);
    m_frontend.clear(clearColor);
    auto viewMatrix = CameraManager::processViewMatrix(camTransform);
    m_frontend.setViewMatrix(viewMatrix);
}

void Renderer::beginSceneRender(const vec3& camPosition, const vec3& camDirection, const vec3& camUp)
{
    auto clearColor = math::vec4(0.2, 0.2, 0.25, 1.0);
    m_frontend.clear(clearColor);
    auto viewMatrix = lookAt(camPosition, camPosition + camDirection, camUp);
    m_frontend.setViewMatrix(viewMatrix);
}

void Renderer::endSceneRender()
{
    m_frontend.unbindFrameBuffer(m_sceneFrameBufferID);
    m_frontend.dumpCommandsToBuffer();
}

void Renderer::applyPostProcessing()
{
    m_frontend.changeUsedProgram(ProgramType::POST_PROCESSING);
    m_frontend.initPostProcessing(m_sceneFrameBufferID);
    m_frontend.submit(m_postProcessingQuadID);
    m_frontend.changeUsedProgram(ProgramType::PBR);
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
    switchCommandBuffer();
    m_commandBuffers[m_frontCommandBufferIdx].clear();

    std::function<void()> prgToUse;
    switch (filter) {
    case FilterEnum::IRRADIANCE:
        prgToUse = [this]() {
            m_frontend.changeUsedProgram(ProgramType::FILTER_IRRADIANCE);
        };
        break;
    default:
        GLX_CORE_ERROR("Unknown filter applied!");
        return;
    }

    Cubemap& targetCubemap = *m_backend.m_cubemapInstances.get(targetID);
    targetCubemap.useFloat = true;
    targetCubemap.resize(2048);
    CubemapFrameBuffer cubemapBuffer(targetCubemap);

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glViewport(0, 0, targetCubemap.resolution, targetCubemap.resolution);

    mat4 baseProjection = m_frontend.getProjectionMatrix();
    mat4 projection     = perspective(radians(90.0f), 1.f, 0.001f, 9999.f);
    m_frontend.setProjectionMatrix(projection);

    vec3 position = vec3(0, 0, 0);
    Transform transformTemp;
    for (int i = 0; i < 6; i++) {
        cubemapBuffer.bind(i);
        beginSceneRender(position, m_cubemap_orientations[i], m_cubemap_ups[i]);
        prgToUse();
        m_frontend.bindCubemap(sourceID, "skybox");
        m_frontend.submit(skyboxMesh, transformTemp);
        endSceneRender();
        renderFrame();
    }
    cubemapBuffer.unbind();
    cubemapBuffer.destroy();

    m_frontend.setProjectionMatrix(baseProjection);
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

void Renderer::renderFromPoint(vec3 position, Node& root, renderID targetCubemapID)
{
    switchCommandBuffer();
    m_commandBuffers[m_frontCommandBufferIdx].clear();

    Cubemap& targetCubemap = *m_backend.m_cubemapInstances.get(targetCubemapID);
    targetCubemap.useFloat = true;
    targetCubemap.resize(2048);
    CubemapFrameBuffer cubemapBuffer(targetCubemap);

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glViewport(0, 0, targetCubemap.resolution, targetCubemap.resolution);

    mat4 baseProjection = m_frontend.getProjectionMatrix();
    mat4 projection     = perspective(radians(90.0f), 1.f, 0.001f, 9999.f);
    m_frontend.setProjectionMatrix(projection);

    for (int i = 0; i < 6; i++) {
        cubemapBuffer.bind(i);
        beginSceneRender(position, m_cubemap_orientations[i], m_cubemap_ups[i]);
        root.draw();
        endSceneRender();
        renderFrame();
    }
    cubemapBuffer.unbind();
    cubemapBuffer.destroy();

    m_frontend.setProjectionMatrix(baseProjection);
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}
}
