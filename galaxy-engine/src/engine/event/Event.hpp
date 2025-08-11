#pragma once

#include "Core.hpp"

namespace Galaxy {
enum class EventType {
    None = 0,
    WindowClose,
    WindowResize,
    KeyInteract,
    MouseButtonInteract,
    MouseMotion,
    ActionInteract
};
enum EventCategory {
    None                     = 0,
    EventCategoryApplication = BIT(0),
    EventCategoryInput       = BIT(1),
    EventCategoryKeyboard    = BIT(2),
    EventCategoryMouse       = BIT(3),
    EventCategoryAction      = BIT(4)
};

// maccro to simplify Event implementation writting (copied from the cherno tutorial but I don't know if it is a good practice and easy to understand)
#define EVENT_CLASS_TYPE(type)                                                  \
    static EventType getStaticType() { return EventType::type; }                \
    virtual EventType getEventType() const override { return getStaticType(); } \
    virtual const char* getName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) \
    virtual int getCategoryFlags() const override { return category; }

class EventDispatcher;

class Event {
    friend EventDispatcher;

private:
    bool m_captured = false;

public:
    bool isCaptured() const { return m_captured; };
    inline void capture() { m_captured = true; }

    virtual EventType getEventType() const = 0;
    virtual const char* getName() const    = 0;
    virtual int getCategoryFlags() const   = 0;
    inline bool isInCategory(EventCategory category) const
    {
        return getCategoryFlags() & category;
    }
};

class EventDispatcher {
    template <typename T>
    using EventFn = std::function<bool(T&)>;

public:
    EventDispatcher(Event& event)
        : m_event(event)
    {
    }

    template <typename T>
    bool dispatch(EventFn<T> func)
    {
        if (m_event.getEventType() == T::getStaticType()) {
            m_event.m_captured = func(*(T*)&m_event);
            return true;
        }
        return false;
    }

private:
    Event& m_event;
};
}
