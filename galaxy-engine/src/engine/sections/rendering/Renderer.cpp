#include "Renderer.hpp"

#include "pch.hpp"

#include "CameraManager.hpp"
#include "Core.hpp"

namespace Galaxy {
Renderer::Renderer()
    : m_commandBuffers(2)
    , m_frontCommandBufferIdx(0)
    , m_frontend(m_commandBuffers[m_frontCommandBufferIdx])
    , m_backend()
{
}

Renderer::~Renderer()
{
}

Renderer& Renderer::getInstance()
{
    static Renderer renderer;
    return renderer;
}

void Renderer::beginSceneRender(mat4& camTransform)
{
    auto clearColor = math::vec4(0.2, 0.2, 0.25, 1.0);
    m_frontend.clear(clearColor);
    auto viewMatrix = CameraManager::processViewMatrix(camTransform);
    m_frontend.setViewMatrix(viewMatrix);
}

void Renderer::submit(renderID meshID, const Transform& transform)
{
    m_frontend.submit(meshID, transform);
}

void Renderer::endSceneRender()
{
}

void Renderer::renderFrame()
{
    m_backend.processCommands(m_commandBuffers[m_frontCommandBufferIdx]);
    m_commandBuffers[m_frontCommandBufferIdx].clear();
    m_frontCommandBufferIdx = 1 - m_frontCommandBufferIdx;
    m_frontend.setCommandBuffer(m_commandBuffers[m_frontCommandBufferIdx]);
}

renderID Renderer::instanciateMesh(std::vector<Vertex>& vertices, std::vector<short unsigned int>& indices)
{
    return m_backend.instanciateMesh(vertices, indices);
}

renderID Renderer::instanciateMesh(ResourceHandle<Mesh> mesh)
{
    return m_backend.instanciateMesh(mesh);
}

void Renderer::clearMesh(renderID meshID)
{
    m_backend.clearMesh(meshID);
}
}
