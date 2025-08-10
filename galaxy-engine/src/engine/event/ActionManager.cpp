#include "pch.hpp"

#include "ActionManager.hpp"

#include <GLFW/glfw3.h>

namespace Galaxy {
void ActionManager::emitEvent(const EventAction& input)
{
    for (auto& listener : m_listeners) {
        listener(input);
    }
}

void ActionManager::addAction(Action action)
{
    m_managedActionsByKey[action.m_glfwKey].push_back(action);
    m_actionKeyByName[action.m_name] = action.m_glfwKey;
}

ActionManager::ActionManager()
{
    // Action forward(GLFW_KEY_W, "forward");
    addAction(Action(GLFW_KEY_W, "forward"));
    addAction(Action(GLFW_KEY_S, "backward"));
    addAction(Action(GLFW_KEY_A, "right"));
    addAction(Action(GLFW_KEY_D, "left"));
    addAction(Action(GLFW_KEY_ESCAPE, "exit"));
}

void ActionManager::processInput(int key, bool pressed)
{
    for (auto& action : m_managedActionsByKey[key]) {
        if (pressed) {
            if (action.m_pressed)
                action.m_clicked = false;
            else {
                action.m_pressed = true;
                action.m_clicked = true;
            }
        } else {
            action.m_pressed = false;
        }

        EventAction inputA(action);
        emitEvent(inputA);
    }
}

void ActionManager::addListener(std::function<void(EventAction inputAction)> listener)
{
    m_listeners.push_back(listener);
}

bool ActionManager::isActionPressed(const std::string& name)
{
    return m_managedActionsByKey[m_actionKeyByName[name]][0].m_pressed;
}
}
