#include <iostream>

#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

#include <backend/imgui_impl_glfw.h>

#include <src/sections/rendering/Renderer.hpp>
#include <src/nodes/MeshInstance.hpp>

int main(int argc, char const *argv[])
{
    Renderer& renderer = Renderer::getInstance();

    glfwSetInputMode(renderer.window, GLFW_STICKY_KEYS, GL_TRUE);

    glfwPollEvents();
    glfwSetCursorPos(renderer.window, 1024/2, 768/2);

    MeshInstance testInstance;
    testInstance.generateTriangle();
    do {
        renderer.renderFrame();
    } while(glfwWindowShouldClose(renderer.window) == 0);

    return 0;
}
