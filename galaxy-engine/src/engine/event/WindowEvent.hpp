#pragma once

#include "Event.hpp"

namespace Galaxy {
class WindowCloseEvent : public Event {
public:
    EVENT_CLASS_CATEGORY(EventCategoryApplication)
    EVENT_CLASS_TYPE(WindowClose)

    WindowCloseEvent() { }
};

class WindowResizeEvent : public Event {
public:
    EVENT_CLASS_CATEGORY(EventCategoryApplication)
    EVENT_CLASS_TYPE(WindowResize)

    WindowResizeEvent(const unsigned int newWidth, const unsigned int newHeight)
        : m_width(newWidth)
        , m_height(newHeight)
    {
    }

    unsigned int getWidth() const { return m_width; }
    unsigned int getHeight() const { return m_height; }

private:
    unsigned int m_width;
    unsigned int m_height;
};

}