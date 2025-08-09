#pragma once

#include "Core.hpp"
#include "pch.hpp"

namespace Galaxy {
struct WindowProps {
    std::string Title;
    unsigned int Width;
    unsigned int Height;

    WindowProps(const std::string& title = "Galaxy Engine",
        unsigned int width = 1280,
        unsigned int height = 720)
        : Title(title)
        , Width(width)
        , Height(height)
    {
    }
    std::function<void(int key, bool pressed)> KeyCallback;
};

class API Window {
public:
    virtual ~Window() {};
    virtual void onUpdate() {};

    virtual unsigned int getWidth() const = 0;
    virtual unsigned int getHeight() const = 0;

    static Window* create(const WindowProps& props = WindowProps());

public:
};
} // namespace Galaxy