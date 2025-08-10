#pragma once

#include "Event.hpp"

namespace Galaxy {
struct Action {
    std::string m_name;
    int m_glfwKey;
    bool m_pressed;
    bool m_clicked;

    Action(int key, std::string name);
};

class EventAction : public Event {
public:
    EventAction(Action act)
        : action(act) {};
    Action action;
};
}
