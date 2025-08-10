#include "pch.hpp"

#include "Camera.hpp"

#include "engine/sections/rendering/Renderer.hpp"

namespace Galaxy {
Camera::Camera()
{
    m_current = true;
    id = Renderer::getInstance().addCamera(getTransform());
    Renderer::getInstance().setCurrentCamera(id);
}

Camera::~Camera()
{
    Renderer::getInstance().removeCamera(id);
}
void Camera::handleInputFromBot(const EventAction& evtAction)
{
    const float speed = 0.1f;
    if (evtAction.action.m_name == "forward" && evtAction.action.m_pressed) {
        translate(vec3(0, 0, speed));
    } else if (evtAction.action.m_name == "backward" && evtAction.action.m_pressed) {
        translate(vec3(0, 0, -speed));
    } else if (evtAction.action.m_name == "left" && evtAction.action.m_pressed) {
        translate(vec3(-speed, 0, 0));
    } else if (evtAction.action.m_name == "right" && evtAction.action.m_pressed) {
        translate(vec3(speed, 0, 0));
    }
}

void Camera::accept(Galaxy::NodeVisitor& visitor)
{
    visitor.visit(*this);
}
}
