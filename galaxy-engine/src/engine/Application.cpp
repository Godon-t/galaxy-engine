#include "Application.hpp"

#include "gl_headers.hpp"

#include "pch.hpp"

// Include GLFW

#include "Log.hpp"
#include "engine/event/WindowEvent.hpp"
#include "engine/nodes/Root.hpp"
#include "engine/nodes/rendering/Camera.hpp"
#include "engine/nodes/rendering/MeshInstance.hpp"
#include "engine/sections/rendering/Renderer.hpp"

namespace Galaxy {
Application* Application::s_instance = nullptr;
Application::Application()
{
    GLX_CORE_ASSERT(!s_instance, "Application already exist");
    s_instance = this;

    WindowProps props = WindowProps();
    actionManager     = std::make_unique<ActionManager>();
    m_terminated      = false;
    m_window          = std::unique_ptr<Window>(Window::create(props));
    m_layerStack      = LayerStack();

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
    m_terminated       = false;
    Renderer& renderer = Renderer::getInstance();

    std::unique_ptr<MeshInstance> testInstance = std::make_unique<MeshInstance>();
    testInstance->generateTriangle();
    testInstance->translate(vec3(0, 0, 2));

    std::unique_ptr<Camera> mainCam = std::make_unique<Camera>();
    testInstance->addChild(std::move(mainCam));

    Root root(*actionManager, std::move(testInstance));

    actionManager->addListener([this](ActionEvent inputAction) {
        m_terminated = std::string(inputAction.getName()) == "exit";
    });

    do {
        for (Layer* layer : m_layerStack) {
            layer->onUpdate();
        }

        root.process();

        m_window->onUpdate();
        renderer.renderFrame();
    } while (!m_terminated);
}
}
