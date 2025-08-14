#include "Application.hpp"

#include "gl_headers.hpp"

#include "pch.hpp"

#include <bits/this_thread_sleep.h>

#include "Log.hpp"
#include "engine/event/WindowEvent.hpp"
#include "engine/nodes/rendering/Camera.hpp"
#include "engine/nodes/rendering/MeshInstance.hpp"
#include "engine/sections/rendering/Renderer.hpp"

namespace Galaxy {
using clock = std::chrono::high_resolution_clock;

Application* Application::s_instance = nullptr;

Application::Application()
{
    GLX_CORE_ASSERT(!s_instance, "Application already exist");
    s_instance = this;

    WindowProps props = WindowProps();
    actionManager     = std::make_unique<ActionManager>();

    m_root          = std::make_unique<Root>();
    m_terminated    = false;
    m_window        = std::unique_ptr<Window>(Window::create(props));
    m_layerStack    = LayerStack();
    m_frameDuration = std::chrono::milliseconds(1000 / 60); // 60 fps
    m_imGuiLayer    = new ImGuiLayer();
    pushOverlay(m_imGuiLayer);

    m_window->setEventCallback([this](Event& event) {
        for (auto it = m_layerStack.end(); it != m_layerStack.begin();) {
            (*--it)->onEvent(event);
            if (event.isCaptured())
                break;
        }

        if (event.isInCategory(EventCategory::EventCategoryKeyboard)) {
            actionManager->processInput((KeyEvent&)event);
        } else if (event.isInCategory(EventCategory::EventCategoryApplication)) {
            if (event.getEventType() == EventType::WindowClose) {
                m_terminated = true;
            } else if (event.getEventType() == EventType::WindowResize) {
                WindowResizeEvent& windowResize = (WindowResizeEvent&)event;
                GLX_CORE_INFO("Window resize ({0}, {1})", windowResize.getWidth(), windowResize.getHeight());
            }
        }
    });
}

Application::~Application() { }

void Application::pushLayer(Layer* layer)
{
    m_layerStack.pushLayer(layer);
    layer->onAttach();
}
void Application::pushOverlay(Layer* overlay)
{
    m_layerStack.pushOverlay(overlay);
    overlay->onAttach();
}

void Application::run()
{
    Renderer& renderer = Renderer::getInstance();

    actionManager->addListener([this](ActionEvent inputAction) {
        m_root->handleEvent(inputAction);
        m_terminated = std::string(inputAction.getActionName()) == "exit";
    });

    m_delta       = 0;
    auto lastTime = clock::now();
    do {
        auto startTime = clock::now();
        m_delta        = std::chrono::duration<double>(startTime - lastTime).count();
        lastTime       = startTime;

        for (Layer* layer : m_layerStack) {
            layer->onUpdate();
        }

        m_imGuiLayer->begin();
        for (Layer* layer : m_layerStack) {
            layer->onImGuiRender();
        }
        m_imGuiLayer->end();

        m_window->onUpdate();
        auto cameraTransform = CameraManager::getInstance().getCurrentCamTransform();
        renderer.beginSceneRender(cameraTransform);

        m_root->process(m_delta);

        renderer.renderFrame();

        // auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(startTime - clock::now());
        // if (elapsed < m_frameDuration)
        //     std::this_thread::sleep_for(m_frameDuration - elapsed);

    } while (!m_terminated);
}
void Application::setRootNode(std::shared_ptr<Node> node)
{
    m_root->setRoot(node);
}
std::shared_ptr<Node> Application::getRootNodePtr()
{
    return m_root->getNodePtr();
}
}
