#include "pch.hpp"

#include "Camera.hpp"

#include "engine/sections/rendering/Renderer.hpp"

namespace Galaxy {
Camera::Camera()
{
    m_current = true;
    id        = Renderer::getInstance().addCamera(getTransform());
    Renderer::getInstance().setCurrentCamera(id);
}

Camera::~Camera()
{
    Renderer::getInstance().removeCamera(id);
}
void Camera::handleInputFromBot(const ActionEvent& evtAction)
{
    const float speed = 0.1f;
    auto actionName   = std::string(evtAction.getName());
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
