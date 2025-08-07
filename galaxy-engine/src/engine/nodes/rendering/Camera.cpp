#include "pch.hpp"

#include "Camera.hpp"

#include "engine/sections/rendering/Renderer.hpp"

Camera::Camera(){
    m_current = true;
    id = Renderer::getInstance().addCamera(getTransform());
    Renderer::getInstance().setCurrentCamera(id);
}

Camera::~Camera(){
    Renderer::getInstance().removeCamera(id);
}
void Camera::handleInputFromBot(const InputAction& inputAction)
{
    const float speed = 0.1f;
    if(inputAction.action.m_name == "forward" && inputAction.action.m_pressed){
        translate(vec3(0,0,speed));
    } else if(inputAction.action.m_name == "backward" && inputAction.action.m_pressed){
        translate(vec3(0,0,-speed));
    } else if(inputAction.action.m_name == "left" && inputAction.action.m_pressed){
        translate(vec3(-speed,0,0));
    } else if(inputAction.action.m_name == "right" && inputAction.action.m_pressed){
        translate(vec3(speed,0,0));
    }
}

void Camera::accept(Galaxy::NodeVisitor &visitor)
{
    visitor.visit(*this);
}
