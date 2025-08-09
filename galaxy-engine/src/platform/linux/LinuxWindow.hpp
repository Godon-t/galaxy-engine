#pragma once

#include "engine/Window.hpp"

#include <GLFW/glfw3.h>

namespace Galaxy {
class LinuxWindow : public Window {
public:
    LinuxWindow(const WindowProps& props);
    virtual ~LinuxWindow();

    void onUpdate() override;

    inline unsigned int getWidth() const override { return m_data.Width; }
    inline unsigned int getHeight() const override { return m_data.Height; }

private:
    void init(const WindowProps& props);
    void shutdown();

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void key_input_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

private:
    GLFWwindow* m_window;

    struct WindowData {
        std::string Title;
        unsigned int Width, Height;
        std::function<void(int key, bool pressed)> KeyCallback;
    };

    WindowData m_data;
};
}