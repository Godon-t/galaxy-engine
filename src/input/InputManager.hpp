#pragma once

#include <functional>
#include <GLFW/glfw3.h>

#include <src/input/Action.hpp>

class InputManager{
private:
    GLFWwindow* context;
    std::vector<Action> managedActions;
    std::vector<std::function<void(InputAction inputAction)>> listeners;
    void emitEvent(const InputAction& input);
public:
    InputManager(GLFWwindow* window): context(window){
        Action forward(GLFW_KEY_W, "forward");
        Action backward(GLFW_KEY_S, "backward");
        Action right(GLFW_KEY_A, "right");
        Action left(GLFW_KEY_D, "left");
        
        Action exit(GLFW_KEY_ESCAPE, "exit");

        managedActions.push_back(exit);
        
        managedActions.push_back(forward);
        managedActions.push_back(backward);
        managedActions.push_back(right);
        managedActions.push_back(left);
    }; 
    void processInputs();
    void addListener(std::function<void(InputAction inputAction)> listener);
};
