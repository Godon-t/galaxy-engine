#include "pch.hpp"

#include "CameraNode.hpp"

#include "rendering/CameraManager.hpp"
#include "rendering/renderer/Renderer.hpp"
#include "event/WindowEvent.hpp"

#include "data/Camera.hpp"


namespace Galaxy {
CameraNode::CameraNode(std::string name)
    : Node3D(name)
{
    m_current = true;
    id        = CameraManager::getInstance().registerCam(m_cameraData);
    CameraManager::getInstance().updateCurrent(id, m_current);
    m_cameraData = std::make_shared<Camera>();
    m_cameraData->dimmensions = Renderer::getInstance().getRenderingWindowSize();
}

CameraNode::~CameraNode()
{
    CameraManager::getInstance().unregisterCam(id);
}

void CameraNode::accept(Galaxy::NodeVisitor& visitor)
{
    visitor.visit(*this);
}
void CameraNode::updateTransformAndChilds(const mat4& matrix)
{
    Node3D::updateTransformAndChilds(matrix);

    auto transfo = m_transform.getGlobalModelMatrix();
    m_cameraData->forward  = vec3(transfo[2][0], transfo[2][1], transfo[2][2]);
    m_cameraData->position = m_transform.getGlobalPosition();
}
void CameraNode::forceUpdateTransformAndChilds(const mat4& matrix)
{
    Node3D::forceUpdateTransformAndChilds(matrix);

    auto transfo = m_transform.getGlobalModelMatrix();
    m_cameraData->forward  = vec3(transfo[2][0], transfo[2][1], transfo[2][2]);
    m_cameraData->position = m_transform.getGlobalPosition();
}
void CameraNode::setCurrent(bool state)
{
    CameraManager::getInstance().updateCurrent(id, state);
    m_current = state;
}
bool CameraNode::getCurrent()
{
    return m_current;
}
vec3 CameraNode::getForward() const
{
    vec3 fwd = vec3(0, 0, -1);
    return normalize(getTransform()->getLocalRotationQuat() * fwd);
}
const std::shared_ptr<Camera> CameraNode::getCamera()
{
    return m_cameraData;
}
void CameraNode::handleInput(const Event& event)
{
    Node3D::handleInput(event);

    m_cameraData->dimmensions = Renderer::getInstance().getRenderingWindowSize();
    // if(event.getEventType() == EventType::WindowResize){
    //     // WindowResizeEvent& windowResize = (WindowResizeEvent&)event;
    //     // m_cameraData->dimmensions = vec2(windowResize.getWidth(), windowResize.getHeight());
    // }
}
}
