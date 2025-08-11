#pragma once

#include "engine/gl_headers.hpp"

#include "CameraManager.hpp"
#include "Program.hpp"
#include "VisualInstance.hpp"

#include "engine/data/Transform.hpp"
#include "engine/types/Render.hpp"

namespace Galaxy {
class Renderer {
private:
    Program m_mainProgram;

    mat4 m_viewMatrix;

    // TODO: Doesn't work anymore with how the renderer work (pointless)
    std::vector<VisualInstance> m_visuInstances;
    size_t instanceCount = 0; // Idx of the last added element
    std::unordered_map<renderID, size_t> m_instanceIdToVisuIdx;
    std::unordered_map<size_t, renderID> m_visuIdxToInstanceId;
    std::stack<renderID> m_freeIds;

private:
    Renderer();
    ~Renderer();

public:
    static Renderer& getInstance();

    void beginSceneRender(mat4& camTransform);
    void submit(const Transform& transform, renderID meshID);
    void endSceneRender();
    void renderFrame();

    renderID instanciateMesh(std::vector<Vertex>& vertices, std::vector<short unsigned int>& indices);
    void clearMesh(renderID meshID);
};
}
