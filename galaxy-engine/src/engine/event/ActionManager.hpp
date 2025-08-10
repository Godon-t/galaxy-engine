#pragma once

#include "pch.hpp"

#include "Action.hpp"

namespace Galaxy {
class ActionManager {
private:
    std::unordered_map<int, std::vector<Action>> m_managedActionsByKey;
    std::unordered_map<std::string, int> m_actionKeyByName;
    std::vector<std::function<void(EventAction inputAction)>> m_listeners;
    void emitEvent(const EventAction& input);
    void addAction(Action action);

public:
    ActionManager();
    void processInput(int key, bool pressed);
    void addListener(std::function<void(EventAction inputAction)> listener);
    bool isActionPressed(const std::string& name);
};
}
