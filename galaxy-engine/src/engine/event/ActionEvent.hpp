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

class ActionEvent : public KeyEvent {
public:
    ActionEvent(int key, bool pressed, bool clicked, std::string& name)
        : KeyEvent(key, pressed, clicked)
    {
        m_name = name.c_str();
    }
    inline const char* getName() const { return m_name; }

private:
    const char* m_name;
};
}
