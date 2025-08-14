#pragma once

#include "Window.hpp"

#include <GLFW/glfw3.h>

namespace Galaxy {
class LinuxWindow : public Window {
public:
    LinuxWindow(const WindowProps& props);
    virtual ~LinuxWindow();

    void onUpdate() override;

    inline unsigned int getWidth() const override { return m_data.Width; }
    inline unsigned int getHeight() const override { return m_data.Height; }

    inline void setEventCallback(const EventCallbackFn& callback) override { m_data.EventCallback = callback; }

    inline void* getNativeWindow() override { return m_window; }

private:
    void init(const WindowProps& props);
    void shutdown();

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void key_input_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void char_callback(GLFWwindow* window, unsigned int codePoint);
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void mouse_motion_callback(GLFWwindow* window, double x, double y);
    static void window_close_callback(GLFWwindow* window);

private:
    GLFWwindow* m_window;

    struct WindowData {
        std::string Title;
        unsigned int Width, Height;
        EventCallbackFn EventCallback;
        std::unordered_map<int, bool> KeyStates;
    };

    WindowData m_data;
};
}