#pragma once

#include "Core.hpp"
#include "Layer.hpp"
#include "LayerStack.hpp"
#include "Window.hpp"
#include "engine/ImGui/ImGuiLayer.hpp"
#include "engine/nodes/Root.hpp"

#include <chrono>

namespace Galaxy {
class API Application {
public:
    Application();
    virtual ~Application();

    void pushLayer(Layer* layer);
    void pushOverlay(Layer* overlay);

    void run();
    void terminate();

    static inline Application& getInstance() { return *s_instance; }
    inline Window& getWindow() { return *m_window; }

    void setRootNode(std::shared_ptr<Node> node);
    std::shared_ptr<Node> getRootNodePtr();
    inline std::shared_ptr<Root> getRoot() const { return m_root; }

    inline double getDelta() { return m_delta; }

private:
    static Application* s_instance;
    ImGuiLayer* m_imGuiLayer;
    std::unique_ptr<Window> m_window;
    LayerStack m_layerStack;
    bool m_terminated;

    double m_delta;
    std::chrono::milliseconds m_frameDuration;

    std::shared_ptr<Root> m_root;
};

Application* createApplication(int argc, char const* argv[]);
} // namespace Galaxy
