#include "Application.hpp"

#include <GL/glew.h>

#include "pch.hpp"

// Include GLFW
#include <GLFW/glfw3.h>
#include <backend/imgui_impl_glfw.h>

#include "engine/input/InputManager.hpp"
#include "engine/nodes/Root.hpp"
#include "engine/nodes/rendering/Camera.hpp"
#include "engine/nodes/rendering/MeshInstance.hpp"
#include "engine/sections/rendering/Renderer.hpp"

Galaxy::Application::Application() { }

Galaxy::Application::~Application() { }

void Galaxy::Application::run()
{
    Renderer& renderer = Renderer::getInstance();

    glfwSetInputMode(renderer.window, GLFW_STICKY_KEYS, GL_TRUE);

    glfwPollEvents();
    glfwSetCursorPos(renderer.window, 1024 / 2, 768 / 2);

    InputManager inputManager(renderer.window);

    std::unique_ptr<MeshInstance> testInstance = std::make_unique<MeshInstance>();
    testInstance->generateTriangle();
    testInstance->translate(vec3(0, 0, 2));

    std::unique_ptr<Camera> mainCam = std::make_unique<Camera>();
    testInstance->addChild(std::move(mainCam));

    Root root(inputManager, std::move(testInstance));

    bool terminated = false;
    inputManager.addListener([&terminated](InputAction inputAction) {
        terminated = inputAction.action.m_name == "exit";
    });

    do {
        inputManager.processInputs();

        root.process();

        renderer.renderFrame();
    } while (!terminated);
}
