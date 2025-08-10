#pragma once

#include "Core.hpp"
#include "Layer.hpp"
#include "LayerStack.hpp"
#include "Window.hpp"
#include "event/ActionManager.hpp"

namespace Galaxy {
class API Application {
public:
    Application();
    virtual ~Application();

    void pushLayer(Layer* layer);
    void pushOverlay(Layer* overlay);

    void run();

    // TODO: find a better place
    std::unique_ptr<ActionManager> actionManager;

    // TODO: need an event dispatch

private:
    std::unique_ptr<Window> m_window;
    LayerStack m_layerStack;
};

Application* createApplication();
} // namespace Galaxy
