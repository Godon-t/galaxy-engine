#pragma once

#include "Event.hpp"

namespace Galaxy {
class KeyEvent : public Event {
public:
    inline int getKeyCode() const { return m_keyCode; }
    inline bool isPressed() const { return m_pressed; }
    inline bool isClicked() const { return m_clicked; }

    EVENT_CLASS_TYPE(KeyInteract)
    EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryKeyboard)

    KeyEvent(int keyCode, bool pressed, bool clicked)
        : m_keyCode(keyCode)
        , m_pressed(pressed)
        , m_clicked(clicked)
    {
    }

private:
    int m_keyCode;
    bool m_pressed;
    bool m_clicked;
};

class CharEvent : public Event {
public:
    inline unsigned int getCodePoint() const { return m_codePoint; }

    EVENT_CLASS_TYPE(CharEmit)
    EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryChar)

    CharEvent(unsigned int codePoint)
        : m_codePoint(codePoint)
    {
    }

private:
    unsigned int m_codePoint;
};

}