#include "pch.hpp"

#include "Camera.hpp"

#include "engine/sections/rendering/Renderer.hpp"

namespace Galaxy {
Camera::Camera(std::string name)
    : Node3D(name)
{
    m_current = true;
    id        = CameraManager::getInstance().registerCam(getTransform());
    CameraManager::getInstance().setCurrent(id);
}

Camera::~Camera()
{
    CameraManager::getInstance().unregisterCam(id);
}
void Camera::handleInputFromBot(const Event& event)
{
    if (!event.isInCategory(EventCategory::EventCategoryAction))
        return;

    const ActionEvent& evtAction = (ActionEvent&)event;
    const float speed            = 0.1f;
    auto actionName              = std::string(evtAction.getActionName());
    if (actionName == "forward" && evtAction.isPressed()) {
        translate(vec3(0, 0, speed));
    } else if (actionName == "backward" && evtAction.isPressed()) {
        translate(vec3(0, 0, -speed));
    } else if (actionName == "left" && evtAction.isPressed()) {
        translate(vec3(-speed, 0, 0));
    } else if (actionName == "right" && evtAction.isPressed()) {
        translate(vec3(speed, 0, 0));
    }
}

void Camera::accept(Galaxy::NodeVisitor& visitor)
{
    visitor.visit(*this);
}
}
