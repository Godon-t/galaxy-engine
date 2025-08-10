#include "pch.hpp"

#include "ActionManager.hpp"
#include "KeyEvent.hpp"
#include <GLFW/glfw3.h>

namespace Galaxy {
void ActionManager::emitEvent(const ActionEvent& input)
{
    for (auto& listener : m_listeners) {
        listener(input);
    }
}

void ActionManager::addAction(Action action)
{
    m_managedActionsByKey[action.key].push_back(action);
    m_actionKeyByName[action.name] = action.key;
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

void ActionManager::processInput(KeyEvent& keyEvent)
{
    int key = keyEvent.getKeyCode();
    GLX_CORE_ERROR("{0}", keyEvent.getKeyCode());
    auto it = m_managedActionsByKey.find(key);
    if (it != m_managedActionsByKey.end()) {
        for (auto& action : m_managedActionsByKey[key]) {
            ActionEvent inputA(key, keyEvent.isPressed(), keyEvent.isClicked(), action.name);
            emitEvent(inputA);
        }
    }
}

void ActionManager::addListener(std::function<void(ActionEvent inputAction)> listener)
{
    m_listeners.push_back(listener);
}

bool ActionManager::isActionPressed(const std::string& name)
{
    return false;
    // TODO
    //  return m_managedActionsByKey[m_actionKeyByName[name]][0].m_pressed;
}
}
