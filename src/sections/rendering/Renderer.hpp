#pragma once

#include <bits/this_thread_sleep.h>
#include <stack>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <src/sections/rendering/Program.hpp>
#include <src/sections/rendering/VisualInstance.hpp>

#include <src/types/Render.hpp>
#include <src/data/Transform.hpp>

class Renderer {
private:
    // int nbFrames = 0;
    std::chrono::milliseconds frameDuration;
    Program mainProgram;
    VisualInstance triangleInstance;


    std::vector<std::pair<VisualInstance, Transform*>> visuInstances;
    size_t instanceCount = 0; // Idx of the last added element
    std::unordered_map<renderID, size_t> instanceIdToVisuIdx;
    std::unordered_map<size_t, renderID> visuIdxToInstanceId;
    std::stack<renderID> freeIds;
public:
    GLFWwindow* window;


private:
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    Renderer();
    ~Renderer();

public:
    static Renderer& getInstance();

    void renderFrame();
    renderID instanciateMesh(Transform* transformRef, std::vector<Vertex> &vertices, std::vector<short unsigned int> &indices);
    void clearMesh(renderID meshID);
};

