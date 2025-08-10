#include "Application.hpp"

#include "gl_headers.hpp"

#include "pch.hpp"

// Include GLFW

#include "engine/event/ActionManager.hpp"
#include "engine/nodes/Root.hpp"
#include "engine/nodes/rendering/Camera.hpp"
#include "engine/nodes/rendering/MeshInstance.hpp"
#include "engine/sections/rendering/Renderer.hpp"

namespace Galaxy {
Application::Application()
{
    WindowProps props = WindowProps();
    actionManager = std::make_unique<ActionManager>();
    props.KeyCallback = [this](int key, bool pressed) {
        actionManager->processInput(key, pressed);
    };

    m_window = std::unique_ptr<Window>(Window::create(props));

    m_layerStack = LayerStack();
}

Application::~Application() { }

void Application::pushLayer(Layer* layer)
{
    m_layerStack.pushLayer(layer);
}
void Application::pushOverlay(Layer* overlay)
{
    m_layerStack.pushOverlay(overlay);
}

void Application::run()
{
    Renderer& renderer = Renderer::getInstance();

    std::unique_ptr<MeshInstance> testInstance = std::make_unique<MeshInstance>();
    testInstance->generateTriangle();
    testInstance->translate(vec3(0, 0, 2));

    std::unique_ptr<Camera> mainCam = std::make_unique<Camera>();
    testInstance->addChild(std::move(mainCam));

    Root root(*actionManager, std::move(testInstance));

    bool terminated = false;
    actionManager->addListener([&terminated](EventAction inputAction) {
        terminated = inputAction.action.m_name == "exit";
    });

    do {
        for (Layer* layer : m_layerStack) {
            layer->onUpdate();
        }

        root.process();

        renderer.renderFrame();
        m_window->onUpdate();
    } while (!terminated);
}
}
