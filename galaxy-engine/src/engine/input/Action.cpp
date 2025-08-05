#include "pch.hpp"

#include "Action.hpp"

Action::Action(int key, std::string name)
{
    m_glfwKey = key;
    this->m_name = name;
    m_pressed = false;
    m_clicked = false;
}
