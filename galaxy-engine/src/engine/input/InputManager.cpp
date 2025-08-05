#include "pch.hpp"

#include "InputManager.hpp"

void InputManager::emitEvent(const InputAction& input)
{
    for(auto& listener : m_listeners){
        listener(input);
    }
}

void InputManager::processInputs()
{
    for(auto& action : m_managedActions){
        if (glfwGetKey(m_context, action.m_glfwKey) == GLFW_PRESS) {
            if(action.m_pressed) action.m_clicked = false;
            else {
                action.m_pressed = true;
                action.m_clicked = true;
            }
        } else {
            action.m_pressed = false;
        }

        if(action.m_pressed){
            InputAction inputA(action);
            emitEvent(inputA);
        }
    }

    glfwPollEvents();
}

void InputManager::addListener(std::function<void(InputAction inputAction)> listener)
{
    m_listeners.push_back(listener);
}
