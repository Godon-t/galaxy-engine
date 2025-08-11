#pragma once

#include "KeyEvent.hpp"
#include "pch.hpp"

namespace Galaxy {
struct Action {
    std::string name;
    int key;
    Action(int key, std::string name)
        : key(key)
        , name(name)
    {
    }
};

class ActionEvent : public Event {
public:
    ActionEvent(bool pressed, bool clicked, std::string& name)
        : m_pressed(pressed)
        , m_clicked(clicked)
        , m_actionName(name.c_str())
    {
    }

    inline const char* getActionName() const { return m_actionName; }
    inline bool isPressed() const { return m_pressed; }
    inline bool isClicked() const { return m_clicked; }

    EVENT_CLASS_TYPE(ActionInteract)
    EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryAction)

private:
    const char* m_actionName;
    bool m_pressed;
    bool m_clicked;
};
}
