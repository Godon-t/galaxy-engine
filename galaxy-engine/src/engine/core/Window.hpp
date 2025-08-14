#pragma once

#include "Core.hpp"
#include "pch.hpp"

#include "engine/event/Event.hpp"

namespace Galaxy {
struct WindowProps {
    std::string Title;
    unsigned int Width;
    unsigned int Height;

    WindowProps(const std::string& title = "Galaxy Engine",
        unsigned int width               = 1280,
        unsigned int height              = 720)
        : Title(title)
        , Width(width)
        , Height(height)
    {
    }
    std::function<void(int key, bool pressed)> KeyCallback;
};

class API Window {
public:
    using EventCallbackFn = std::function<void(Event&)>;

    virtual ~Window() {};
    virtual void onUpdate() {};

    virtual unsigned int getWidth() const  = 0;
    virtual unsigned int getHeight() const = 0;

    static Window* create(const WindowProps& props = WindowProps());

    virtual void setEventCallback(const EventCallbackFn& callback) = 0;

    virtual void* getNativeWindow() = 0;

public:
};
} // namespace Galaxy