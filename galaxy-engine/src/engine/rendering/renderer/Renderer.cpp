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
    m_sceneFrameBufferID   = m_backend.instanciateFrameBuffer(1920, 1080, FramebufferTextureFormat::DEPTH24STENCIL8);
    m_postProcessingQuadID = m_backend.generateQuad(vec2(2, 2), [] {});
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

void Renderer::beginSceneRender(mat4& camTransform)
{
    m_frontend.bindFrameBuffer(m_sceneFrameBufferID);
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

void Renderer::renderFromPoint(vec3 position, Node& root, renderID targetCubemapID)
{
    switchCommandBuffer();
    m_commandBuffers[m_frontCommandBufferIdx].clear();

    vec3 orientations[6], ups[6];
    orientations[0] = { 1, 0, 0 };
    orientations[1] = { -1, 0, 0 };
    ups[0] = ups[1] = { 0, -1, 0 };

    orientations[2] = { 0, 1, 0 };
    orientations[3] = { 0, -1, 0 };
    ups[2]          = { 0, 0, 1 };
    ups[3]          = { 0, 0, -1 };

    orientations[4] = { 0, 0, 1 };
    orientations[5] = { 0, 0, -1 };
    ups[4] = ups[5] = { 0, -1, 0 };

    Cubemap& targetCubemap = *m_backend.m_cubemapInstances.get(targetCubemapID);
    targetCubemap.useFloat = true;
    targetCubemap.resize(2048);
    CubemapFrameBuffer cubemapBuffer(targetCubemap);

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glViewport(0, 0, targetCubemap.resolution, targetCubemap.resolution);

    mat4 baseProjection = m_frontend.getProjectionMatrix();
    mat4 projection     = perspective(radians(90.0f), 1.f, 0.001f, 999.f);
    m_frontend.setProjectionMatrix(projection);

    for (int i = 0; i < 6; i++) {
        vec4 clearColor = vec4(1, 0, 0, 1);
        cubemapBuffer.bind(i);
        beginSceneRender(position, orientations[i], ups[i]);
        m_frontend.clear(clearColor);
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
