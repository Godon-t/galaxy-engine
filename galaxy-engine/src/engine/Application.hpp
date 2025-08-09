#pragma once

#include "Core.hpp"
#include "Window.hpp"
#include "input/ActionManager.hpp"

namespace Galaxy {
class API Application {
public:
    Application();
    virtual ~Application();

    void run();

    // Defined client side
    std::unique_ptr<ActionManager> actionManager;

private:
    std::unique_ptr<Window> m_window;
};

Application* createApplication();
} // namespace Galaxy
