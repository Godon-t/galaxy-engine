#pragma once

#include "Program.hpp"
#include "RenderCommand.hpp"
#include "VisualInstance.hpp"
#include "engine/resource/Mesh.hpp"
#include "engine/resource/ResourceHandle.hpp"
#include "engine/types/Render.hpp"

namespace Galaxy {
class Backend {
public:
    Backend(size_t maxSize = 512);

    renderID instanciateMesh(ResourceHandle<Mesh> mesh);
    renderID instanciateMesh(std::vector<Vertex>& vertices, std::vector<unsigned short>& indices);
    void clearMesh(renderID meshID);

    void processCommands(std::vector<RenderCommand>& commands);

private:
    void processCommand(RenderCommand& command);

    // TODO: Doesn't work anymore with how the renderer work (pointless)
    size_t instanceCount = 0; // Idx of the last added element
    std::unordered_map<renderID, size_t> m_instanceIdToVisuIdx;
    std::unordered_map<size_t, renderID> m_visuIdxToInstanceId;
    std::stack<renderID> m_freeIds;

    std::unordered_map<size_t, renderID> m_resourceTable;
    std::unordered_map<renderID, ResourceHandle<Mesh>> m_idToResource;

    std::vector<std::pair<VisualInstance, size_t>> m_visuInstances;

    Program m_mainProgram;
};
} // namespace Galaxy
