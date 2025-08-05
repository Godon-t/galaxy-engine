#include "pch.hpp"

#include "Action.hpp"

Action::Action(int key, std::string name)
{
    glfwKey = key;
    this->name = name;
    pressed = false;
    clicked = false;
}
