#include "pch.hpp"

#include "Camera.hpp"

#include "rendering/CameraManager.hpp"
#include "rendering/renderer/Renderer.hpp"

namespace Galaxy {
Camera::Camera(std::string name)
    : Node3D(name)
{
    m_current = true;
    id        = CameraManager::getInstance().registerCam(getTransform());
    CameraManager::getInstance().updateCurrent(id, m_current);
}

Camera::~Camera()
{
    CameraManager::getInstance().unregisterCam(id);
}
void Camera::handleInputFromBot(const Event& event)
{
}

void Camera::accept(Galaxy::NodeVisitor& visitor)
{
    visitor.visit(*this);
}
void Camera::setCurrent(bool state)
{
    CameraManager::getInstance().updateCurrent(id, state);
    m_current = state;
}
vec3 Camera::getForward() const
{
    vec3 fwd = vec3(0, 0, -1);
    return normalize(getTransform()->getLocalRotationQuat() * fwd);
}
}
