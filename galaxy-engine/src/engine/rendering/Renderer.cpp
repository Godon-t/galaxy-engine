#include "Renderer.hpp"

#include "pch.hpp"

#include "Application.hpp"
#include "rendering/frontend/CameraManager.hpp"
#include "rendering/GPUInstances/FrameBuffer.hpp"

namespace Galaxy {
Renderer::Renderer()
    : m_backend()
    , m_graph()
    , m_frontend(m_graph, m_backend)
    , m_mainViewportSize(1024)
{
    m_backend.initDebugCallback();

    m_backend.onMaterialUpdated([this](MaterialHandle material, const MaterialRenderInfo& info) {
        m_frontend.notifyMaterialUpdated(material, info);
    });
    m_backend.onMaterialDestroyed([this](MaterialHandle material) {
        m_frontend.removeMaterialID(material);
    });
}

Renderer::~Renderer()
{
    shutdown();
}

void Renderer::shutdown()
{
    m_backend.destroy();
}

Renderer& Renderer::getInstance()
{
    static Renderer renderer;
    return renderer;
}

void Renderer::init()
{
    m_frontend.getLightManager().init();
}

void Renderer::addMainCamera(const Camera& camera)
{
    Camera mainCamera = camera;
    mainCamera.viewportSize = m_mainViewportSize;
    mainCamera.viewportPosition = vec2(0);
    mainCamera.targetFramebuffer.reset();
    m_frontend.addCamera(std::move(mainCamera));
}

void Renderer::resize(unsigned int width, unsigned int height)
{
    m_mainViewportSize.x = width;
    m_mainViewportSize.y = height;
    m_graph.setViewSize(m_mainViewportSize);
}

void Renderer::renderFrame()
{
    m_graph.prepareResources(m_backend);
    m_frontend.addFrameInvocationsToContext(m_graph);
    m_backend.frameReset();
    m_backend.execute(m_graph);
    
    m_frontend.clearContext();
    m_graph.clearInvocations();

    m_sceneFramebuffer = m_graph.getLastFramebufferHandle();
}

}
