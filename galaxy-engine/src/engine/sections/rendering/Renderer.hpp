#pragma once

#include <bits/this_thread_sleep.h>

#include "engine/gl_headers.hpp"

#include "CameraManager.hpp"
#include "Program.hpp"
#include "VisualInstance.hpp"

#include "engine/data/Transform.hpp"
#include "engine/types/Render.hpp"

namespace Galaxy {
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

private:
    Renderer();
    ~Renderer();

public:
    static Renderer& getInstance();

    void renderFrame();
    camID addCamera(Transform* transformRef);
    void setCurrentCamera(camID id);
    void removeCamera(camID id);
    renderID instanciateMesh(Transform* transformRef, std::vector<Vertex>& vertices, std::vector<short unsigned int>& indices);
    void clearMesh(renderID meshID);
};
}
