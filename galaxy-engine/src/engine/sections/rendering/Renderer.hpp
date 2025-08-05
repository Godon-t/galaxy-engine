#pragma once

#include <bits/this_thread_sleep.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Program.hpp"
#include "VisualInstance.hpp"
#include "CameraManager.hpp"

#include "engine/types/Render.hpp"
#include "engine/data/Transform.hpp"

class Renderer {
private:
    // int nbFrames = 0;
    std::chrono::milliseconds m_frameDuration;
    Program m_mainProgram;

    CameraManager m_camManager;


    std::vector<std::pair<VisualInstance, Transform*>> m_visuInstances;
    size_t instanceCount = 0; // Idx of the last added element
    std::unordered_map<renderID, size_t> m_instanceIdToVisuIdx;
    std::unordered_map<size_t, renderID> m_visuIdxToInstanceId;
    std::stack<renderID> m_freeIds;
public:
    GLFWwindow* window;


private:
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    Renderer();
    ~Renderer();

public:
    static Renderer& getInstance();

    void renderFrame();
    camID addCamera(Transform* transformRef);
    void setCurrentCamera(camID id);
    void removeCamera(camID id);
    renderID instanciateMesh(Transform* transformRef, std::vector<Vertex> &vertices, std::vector<short unsigned int> &indices);
    void clearMesh(renderID meshID);
};

