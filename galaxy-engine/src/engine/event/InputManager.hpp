#pragma once

#include "pch.hpp"

#include "ActionEvent.hpp"

namespace Galaxy {
class InputManager {
private:
    std::unordered_map<int, std::vector<Action>> m_managedActionsByKey;
    std::unordered_map<std::string, int> m_actionKeyByName;
    std::vector<std::function<void(ActionEvent inputAction)>> m_listeners;
    void emitEvent(const ActionEvent& input);

    static InputManager* s_instance;

    InputManager();

public:
    static void addAction(Action action);

    static std::vector<ActionEvent> processInput(KeyEvent& keyEvent);

    static void addListener(std::function<void(ActionEvent inputAction)> listener);

    static bool isActionPressed(const std::string& name);
    static bool isKeyPressed(const int keycode);
};
}
