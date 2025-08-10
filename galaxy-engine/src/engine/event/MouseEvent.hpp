#pragma once

#include "Event.hpp"

const int MOUSE_LEFT  = 0;
const int MOUSE_RIGHT = 1;

namespace Galaxy {
class MouseButtonEvent : public Event {
public:
    inline int getButton() const { return m_button; }
    inline bool isPressed() const { return m_pressed; }
    // inline bool isClicked() const { return m_clicked; }

    EVENT_CLASS_TYPE(MouseButtonInteract)
    EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryMouse)

    MouseButtonEvent(int button, bool pressed)
        : m_button(button)
        , m_pressed(pressed)
    {
    }

private:
    int m_button;
    bool m_pressed;
    // bool m_clicked;
};

class MouseMotionEvent : public Event {
public:
    inline double getX() const { return m_x; }
    inline double getY() const { return m_y; }

    EVENT_CLASS_TYPE(MouseMotion)
    EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryMouse)

    MouseMotionEvent(double x, double y)
        : m_x(x)
        , m_y(y)
    {
    }

private:
    double m_x;
    double m_y;
};
}