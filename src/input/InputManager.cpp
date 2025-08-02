#include "InputManager.hpp"
#include <iostream>


void InputManager::emitEvent(const InputAction& input)
{
    for(auto& listener : listeners){
        listener(input);
    }
}

void InputManager::processInputs()
{
    for(auto& action : managedActions){
        if (glfwGetKey(context, action.glfwKey) == GLFW_PRESS) {
            if(action.pressed) action.clicked = false;
            else {
                action.pressed = true;
                action.clicked = true;
            }
        } else {
            action.pressed = false;
        }

        if(action.pressed){
            InputAction inputA(action);
            emitEvent(inputA);
        }
    }

    glfwPollEvents();
}

void InputManager::addListener(std::function<void(InputAction inputAction)> listener)
{
    listeners.push_back(listener);
}
