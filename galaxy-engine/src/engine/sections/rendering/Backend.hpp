#pragma once

#include "Program.hpp"
#include "RenderCommand.hpp"
#include "VisualInstance.hpp"
#include "engine/types/Render.hpp"

namespace Galaxy {
class Backend {
public:
    Backend(size_t maxSize = 512);

    renderID instanciateMesh(std::vector<Vertex>& vertices, std::vector<short unsigned int>& indices);
    void clearMesh(renderID meshID);

    void processCommands(std::vector<RenderCommand>& commands);

private:
    void processCommand(RenderCommand& command);

    // TODO: Doesn't work anymore with how the renderer work (pointless)
    std::vector<VisualInstance> m_visuInstances;
    size_t instanceCount = 0; // Idx of the last added element
    std::unordered_map<renderID, size_t> m_instanceIdToVisuIdx;
    std::unordered_map<size_t, renderID> m_visuIdxToInstanceId;
    std::stack<renderID> m_freeIds;

    Program m_mainProgram;
};
} // namespace Galaxy
