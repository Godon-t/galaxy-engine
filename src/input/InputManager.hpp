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

        managedActions.push_back(forward);
    }; 
    void processInputs();
    void addListener(std::function<void(InputAction inputAction)> listener);
};
