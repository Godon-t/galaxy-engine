#pragma once

#include "Core.hpp"
#include "ImGui/ImGuiLayer.hpp"
#include "Layer.hpp"
#include "LayerStack.hpp"
#include "Window.hpp"
#include "engine/nodes/Root.hpp"
#include "event/ActionManager.hpp"

#include <chrono>

namespace Galaxy {
class API Application {
public:
    Application();
    virtual ~Application();

    void pushLayer(Layer* layer);
    void pushOverlay(Layer* overlay);

    void run();

    static inline Application& getInstance() { return *s_instance; }
    inline Window& getWindow() { return *m_window; }

    // TODO: find a better place
    std::unique_ptr<ActionManager> actionManager;

    void setRootNode(std::unique_ptr<Node> node);

    inline double getDelta() { return m_delta; }

private:
    static Application* s_instance;
    ImGuiLayer* m_imGuiLayer;
    std::unique_ptr<Window> m_window;
    LayerStack m_layerStack;
    bool m_terminated;

    double m_delta;
    std::chrono::milliseconds m_frameDuration;

    std::unique_ptr<Root> m_root;
};

Application* createApplication();
} // namespace Galaxy
