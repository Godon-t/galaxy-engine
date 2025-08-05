#include "pch.hpp"

#include "Camera.hpp"

#include "engine/sections/rendering/Renderer.hpp"

Camera::Camera(){
    current = true;
    id = Renderer::getInstance().addCamera(getTransform());
    Renderer::getInstance().setCurrentCamera(id);
}

Camera::~Camera(){
    Renderer::getInstance().removeCamera(id);
}
void Camera::handleInputFromBot(const InputAction& inputAction)
{
    const float speed = 0.1f;
    if(inputAction.action.name == "forward" && inputAction.action.pressed){
        translate(vec3(0,0,speed));
    } else if(inputAction.action.name == "backward" && inputAction.action.pressed){
        translate(vec3(0,0,-speed));
    } else if(inputAction.action.name == "left" && inputAction.action.pressed){
        translate(vec3(-speed,0,0));
    } else if(inputAction.action.name == "right" && inputAction.action.pressed){
        translate(vec3(speed,0,0));
    }
}