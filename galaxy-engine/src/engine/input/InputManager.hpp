#pragma once

#include <functional>
#include <GLFW/glfw3.h>

#include "Action.hpp"

class InputManager{
private:
    GLFWwindow* m_context;
    std::vector<Action> m_managedActions;
    std::vector<std::function<void(InputAction inputAction)>> m_listeners;
    void emitEvent(const InputAction& input);
public:
    InputManager(GLFWwindow* window): m_context(window){
        Action forward(GLFW_KEY_W, "forward");
        Action backward(GLFW_KEY_S, "backward");
        Action right(GLFW_KEY_A, "right");
        Action left(GLFW_KEY_D, "left");
        
        Action exit(GLFW_KEY_ESCAPE, "exit");

        m_managedActions.push_back(exit);
        
        m_managedActions.push_back(forward);
        m_managedActions.push_back(backward);
        m_managedActions.push_back(right);
        m_managedActions.push_back(left);
    }; 
    void processInputs();
    void addListener(std::function<void(InputAction inputAction)> listener);
};
