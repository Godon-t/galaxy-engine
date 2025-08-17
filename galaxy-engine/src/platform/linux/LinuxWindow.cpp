#include "LinuxWindow.hpp"

#include "Log.hpp"
#include "engine/event/InputManager.hpp"
#include "engine/event/KeyEvent.hpp"
#include "engine/event/MouseEvent.hpp"
#include "engine/event/WindowEvent.hpp"
#include "pch.hpp"

namespace Galaxy {
static bool s_GLFWInitialized = false;

Window* Window::create(const WindowProps& props)
{
    return new LinuxWindow(props);
}

LinuxWindow::LinuxWindow(const WindowProps& props)
{
    init(props);
}

LinuxWindow::~LinuxWindow()
{
    shutdown();
}

void LinuxWindow::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
    data.Width       = width;
    data.Height      = height;
    glViewport(0, 0, width, height);

    WindowResizeEvent evt(width, height);
    data.EventCallback(evt);
}

void LinuxWindow::key_input_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

    auto it = data.KeyStates.find(key);
    if (it == data.KeyStates.end())
        data.KeyStates[key] = false;

    bool clicked = false;
    bool pressed = false;

    switch (action) {
    case GLFW_PRESS: {
        clicked = data.KeyStates[key] == false;
        if (clicked)
            data.KeyStates[key] = true;

        pressed = true;
        break;
    }
    case GLFW_RELEASE: {
        data.KeyStates[key] = false;
        break;
    }
    case GLFW_REPEAT:
        pressed = true;
        break;
    }

    KeyEvent evt(key, pressed, clicked);
    data.EventCallback(evt);

    auto actionEvents = InputManager::processInput(evt);
    for (auto& actEvt : actionEvents) {
        data.EventCallback(actEvt);
    }
}

void LinuxWindow::char_callback(GLFWwindow* window, unsigned int codePoint)
{
    WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

    CharEvent evt(codePoint);
    data.EventCallback(evt);
}

void LinuxWindow::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
    MouseButtonEvent evt(button, action == GLFW_PRESS);
    data.EventCallback(evt);
}

void LinuxWindow::mouse_motion_callback(GLFWwindow* window, double x, double y)
{
    WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
    MouseMotionEvent evt(x, y);
    data.EventCallback(evt);
}

void LinuxWindow::window_close_callback(GLFWwindow* window)
{
    WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
    WindowCloseEvent evt;
    data.EventCallback(evt);
}

void LinuxWindow::init(const WindowProps& props)
{
    m_data.Title  = props.Title;
    m_data.Width  = props.Width;
    m_data.Height = props.Height;

    GLX_CORE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);

    if (!s_GLFWInitialized) {
        bool success = glfwInit();

        GLX_CORE_ASSERT(success, "Glfw init");

        s_GLFWInitialized = true;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_window = glfwCreateWindow(props.Width, props.Height, props.Title.c_str(), NULL, NULL);
    glfwSetWindowSizeCallback(m_window, framebuffer_size_callback);
    glfwSetWindowUserPointer(m_window, &m_data);

    GLX_CORE_ASSERT(m_window != NULL, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version");

    glfwMakeContextCurrent(m_window);

    glfwSetInputMode(m_window, GLFW_STICKY_KEYS, GL_TRUE);

    glfwSetKeyCallback(m_window, key_input_callback);
    glfwSetCharCallback(m_window, char_callback);
    glfwSetMouseButtonCallback(m_window, mouse_button_callback);
    glfwSetCursorPosCallback(m_window, mouse_motion_callback);
    glfwSetWindowCloseCallback(m_window, window_close_callback);

    glfwSetWindowPos(m_window, m_data.Width / 2, m_data.Height / 2);

    // glfwSetCursorPos(m_window, 1024 / 2, 768 / 2);
}

void LinuxWindow::shutdown()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void LinuxWindow::onUpdate()
{
    glfwPollEvents();
    glfwSwapBuffers(m_window);
}
}