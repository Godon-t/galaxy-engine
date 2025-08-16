#include "pch.hpp"

#include "Application.hpp"
#include "InputManager.hpp"
#include "KeyEvent.hpp"
#include <GLFW/glfw3.h>

namespace Galaxy {
InputManager* InputManager::s_instance = new InputManager();

void InputManager::emitEvent(const ActionEvent& input)
{
    for (auto& listener : s_instance->m_listeners) {
        listener(input);
    }
}

void InputManager::addAction(Action action)
{
    s_instance->m_managedActionsByKey[action.key].push_back(action);
    s_instance->m_actionKeyByName[action.name] = action.key;
}

InputManager::InputManager()
{
}

std::vector<ActionEvent> InputManager::processInput(KeyEvent& keyEvent)
{
    std::vector<ActionEvent> res;

    int key = keyEvent.getKeyCode();
    auto it = s_instance->m_managedActionsByKey.find(key);
    if (it != s_instance->m_managedActionsByKey.end()) {
        // TODO: Size is known in advance, find a solution for resize (making a default constructor for ActionEvent feel weird)
        for (auto& action : s_instance->m_managedActionsByKey[key]) {
            res.push_back(ActionEvent(keyEvent, action.name));
        }
    }
    return res;
}

void InputManager::addListener(std::function<void(ActionEvent inputAction)> listener)
{
    s_instance->m_listeners.push_back(listener);
}

bool InputManager::isActionPressed(const std::string& name)
{
    auto actionIt = s_instance->m_actionKeyByName.find(name);
    if (actionIt == s_instance->m_actionKeyByName.end()) {
        GLX_CORE_ERROR("Unknown action '{0}'", name);
        return false;
    } else {
        auto state = glfwGetKey((GLFWwindow*)Application::getInstance().getWindow().getNativeWindow(), actionIt->second);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }
}
bool InputManager::isKeyPressed(const int keycode)
{
    auto state = glfwGetKey((GLFWwindow*)Application::getInstance().getWindow().getNativeWindow(), keycode);
    return state == GLFW_PRESS || state == GLFW_REPEAT;
}
}
