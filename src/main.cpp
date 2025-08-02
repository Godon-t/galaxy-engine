#include <iostream>

#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

#include <backend/imgui_impl_glfw.h>

#include <src/sections/rendering/Renderer.hpp>
#include <src/nodes/rendering/MeshInstance.hpp>
#include <src/nodes/rendering/Camera.hpp>
#include <src/nodes/Root.hpp>
#include <src/input/InputManager.hpp>

int main(int argc, char const *argv[])
{
    Renderer& renderer = Renderer::getInstance();

    glfwSetInputMode(renderer.window, GLFW_STICKY_KEYS, GL_TRUE);

    glfwPollEvents();
    glfwSetCursorPos(renderer.window, 1024/2, 768/2);

    
    InputManager inputManager(renderer.window);

    std::unique_ptr<MeshInstance> testInstance = std::make_unique<MeshInstance>();
    testInstance->generateTriangle();
    testInstance->translate(vec3(0,0,2));

    std::unique_ptr<Camera> mainCam = std::make_unique<Camera>();
    testInstance->addChild(std::move(mainCam));
    
    Root root(inputManager, std::move(testInstance));
    

    bool terminated = false;
    inputManager.addListener([&terminated](InputAction inputAction){terminated = inputAction.action.name == "exit";});
    
    do {
        inputManager.processInputs();
        
        root.process();

        renderer.renderFrame();

    } while(!terminated);

    return 0;
}
