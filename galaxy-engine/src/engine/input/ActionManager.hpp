#pragma once

#include "pch.hpp"

#include "Action.hpp"

class ActionManager {
private:
    std::unordered_map<int, std::vector<Action>> m_managedActionsByKey;
    std::unordered_map<std::string, int> m_actionKeyByName;
    std::vector<std::function<void(InputAction inputAction)>> m_listeners;
    void emitEvent(const InputAction& input);
    void addAction(Action action);

public:
    ActionManager();
    void processInput(int key, bool pressed);
    void addListener(std::function<void(InputAction inputAction)> listener);
    bool isActionPressed(const std::string& name);
};
