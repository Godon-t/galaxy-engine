#include "LinuxWindow.hpp"

#include "Log.hpp"
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
    WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);
    (*data).Width = width;
    (*data).Height = height;
    glViewport(0, 0, width, height);
}

void LinuxWindow::key_input_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);
    (*data).KeyCallback(key, action == GLFW_PRESS || action != GLFW_RELEASE);
}

void LinuxWindow::init(const WindowProps& props)
{
    m_data.Title = props.Title;
    m_data.Width = props.Width;
    m_data.Height = props.Height;
    m_data.KeyCallback = props.KeyCallback;

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
    // TODO: understand how this work
    glfwSetWindowUserPointer(m_window, &m_data);

    GLX_CORE_ASSERT(m_window != NULL, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version");

    glfwMakeContextCurrent(m_window);

    glfwSetInputMode(m_window, GLFW_STICKY_KEYS, GL_TRUE);

    glfwSetKeyCallback(m_window, key_input_callback);

    // glfwSetCursorPos(m_window, 1024 / 2, 768 / 2);
}

void LinuxWindow::shutdown()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void LinuxWindow::onUpdate()
{
    // float currentFrame = glfwGetTime();
    // deltaTime = currentFrame - lastFrame;
    // lastFrame = currentFrame;

    glfwPollEvents();
    glfwSwapBuffers(m_window);
}
}