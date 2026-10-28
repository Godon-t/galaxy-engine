#include "ProbeNode.hpp"

#include "rendering/renderer/Renderer.hpp"

namespace Galaxy {
ProbeNode::ProbeNode(std::string name)
    : Node3D(name)
{
}

ProbeNode::~ProbeNode()
{
    if (!m_cubemap)
        return;

    auto& backend = Renderer::getInstance().getBackend();
    backend.clearFrameBuffer(m_captureFramebuffer);
    backend.clearCubemap(m_cubemap);
}

void ProbeNode::accept(Galaxy::NodeVisitor& visitor)
{
    visitor.visit(*this);
}

void ProbeNode::capture()
{
    m_capturePending = true;
}

void ProbeNode::draw()
{
    if (m_capturePending) {
        Camera capture;
        capture.position = getTransform().getGlobalPosition();
        capture.viewportSize = vec2(512);
        capture.targetFramebuffer = m_captureFramebuffer;
        capture.frustumCulling = false;
        Renderer::getInstance().getFrontend().addCamera(std::move(capture));
        m_capturePending = false;
        m_hasCapture = true;
    }

    Node3D::draw();
}

void ProbeNode::enteredRoot()
{
    auto& backend = Renderer::getInstance().getBackend();
    m_cubemap = backend.instantiateCubemap(512);
    m_captureFramebuffer = backend.instantiateCubemapFrameBuffer(512);
    backend.attachColorCubemapToFramebuffer(m_cubemap, m_captureFramebuffer, 0);
}
} // namespace Galaxy
