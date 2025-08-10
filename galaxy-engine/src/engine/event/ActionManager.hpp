#pragma once

#include "pch.hpp"

#include "ActionEvent.hpp"

namespace Galaxy {
class ActionManager {
private:
    std::unordered_map<int, std::vector<Action>> m_managedActionsByKey;
    std::unordered_map<std::string, int> m_actionKeyByName;
    std::vector<std::function<void(ActionEvent inputAction)>> m_listeners;
    void emitEvent(const ActionEvent& input);
    void addAction(Action action);

public:
    ActionManager();
    void processInput(KeyEvent& keyEvent);
    void addListener(std::function<void(ActionEvent inputAction)> listener);
    bool isActionPressed(const std::string& name);
};
}
