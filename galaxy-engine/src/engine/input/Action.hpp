#pragma once

#include "Input.hpp"

struct Action {
    std::string m_name;
    int m_glfwKey;
    bool m_pressed;
    bool m_clicked;

    Action(int key, std::string name);
};

class InputAction: Input {
public:
    InputAction(Action act): action(act){};
    Action action;
};