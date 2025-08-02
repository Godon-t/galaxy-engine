#pragma once

#include <string>

#include <src/input/Input.hpp>

struct Action {
    std::string name;
    int glfwKey;
    bool pressed;
    bool clicked;

    Action(int key, std::string name);
};

class InputAction: Input {
public:
    InputAction(Action act): action(act){};
    Action action;
};