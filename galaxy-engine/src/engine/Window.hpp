#pragma once

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
};

class Window { }
} // namespace Galaxy