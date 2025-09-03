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
    inline double getDeltaX() const { return m_deltaX; }
    inline double getY() const { return m_y; }
    inline double getDeltaY() const { return m_deltaY; }

    EVENT_CLASS_TYPE(MouseMotion)
    EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryMouse)

    MouseMotionEvent(double x, double y, double deltaX, double deltaY)
        : m_x(x)
        , m_y(y)
        , m_deltaX(deltaX)
        , m_deltaY(deltaY)
    {
    }

private:
    double m_x;
    double m_deltaX;
    double m_y;
    double m_deltaY;
};

class MouseScrollEvent : public Event {
public:
    MouseScrollEvent(double xOffset, double yOffset)
        : m_xOffset(xOffset)
        , m_yOffset(yOffset)
    {
    }
    double getXOffset() const { return m_xOffset; }
    double getYOffset() const { return m_yOffset; }

    EVENT_CLASS_TYPE(MouseScroll)
    EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryMouse)

private:
    double m_xOffset;
    double m_yOffset;
};
}