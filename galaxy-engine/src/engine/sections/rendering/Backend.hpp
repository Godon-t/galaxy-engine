#pragma once

#include "Program.hpp"
#include "RenderCommand.hpp"
#include "VisualInstance.hpp"
#include "engine/core/Log.hpp"
#include "engine/resource/Mesh.hpp"
#include "engine/resource/ResourceHandle.hpp"
#include "engine/types/Render.hpp"

namespace Galaxy {

template <typename T>
class RenderGpuResourceTable {
public:
    RenderGpuResourceTable(int maxSize = 512)
    {
        m_resourcesInstance.resize(maxSize);
        for (size_t i = 0; i < maxSize; i++) {
            m_freeIds.push(i);
        }
    }

    bool tryRemove(renderID idToRemove)
    {
        m_resourcesInstance[idToRemove].second--;
        if (m_resourcesInstance[idToRemove].second > 0)
            return false;

        m_instanceCount--;
        if (m_instanceCount == 0)
            return false;

        // Was the last use gpuResource, we erase
        size_t idxToDelete = m_renderIdToResourceIdx[idToRemove];

        renderID movedMeshID                 = m_resourceIdxToRenderId[m_instanceCount];
        m_renderIdToResourceIdx[movedMeshID] = idxToDelete;
        m_resourceIdxToRenderId[idxToDelete] = movedMeshID;

        m_resourcesInstance[idxToDelete] = std::move(m_resourcesInstance[m_instanceCount]);

        m_freeIds.emplace(idToRemove);
        return true;
    }

    renderID createResourceInstance()
    {
        if (m_freeIds.size() == 0) {
            GLX_CORE_ERROR("No more free renderIDs");
            return -1;
        }

        renderID createdID = m_freeIds.top();
        size_t listIdx     = m_instanceCount;

        m_resourcesInstance[listIdx]       = std::make_pair(T(), 1);
        m_resourceIdxToRenderId[listIdx]   = createdID;
        m_renderIdToResourceIdx[createdID] = listIdx;

        m_freeIds.pop();
        m_instanceCount++;

        return createdID;
    }

    void increaseCount(renderID id)
    {
        m_resourcesInstance[id].second++;
    }

    T* get(renderID id)
    {
        auto resourceIdx = m_renderIdToResourceIdx[id];
        return &m_resourcesInstance[resourceIdx].first;
    }

    bool canAddInstance() { return m_freeIds.size() > 0; }

private:
    size_t m_instanceCount = 0;
    std::unordered_map<renderID, size_t> m_renderIdToResourceIdx;
    std::unordered_map<size_t, renderID> m_resourceIdxToRenderId;
    std::stack<renderID> m_freeIds;

    std::vector<std::pair<T, size_t>> m_resourcesInstance;
};

class Backend {
public:
    Backend(size_t maxSize = 512);

    renderID instanciateMesh(ResourceHandle<Mesh> mesh, int surfaceIdx);
    renderID instanciateMesh(std::vector<Vertex>& vertices, std::vector<unsigned short>& indices);
    void clearMesh(renderID meshID);

    void processCommands(std::vector<RenderCommand>& commands);

private:
    struct MeshHandle {
        ResourceHandle<Mesh> mesh;
        std::unordered_map<int, renderID> activesSubMeshes;
    };

    void processCommand(RenderCommand& command);

    std::unordered_map<size_t, MeshHandle> m_resourceTable;
    std::unordered_map<renderID, size_t> m_idToResource;

    // std::vector<std::pair<VisualInstance, size_t>> m_visuInstances;

    Program m_mainProgram;

    RenderGpuResourceTable<VisualInstance> m_visualInstances;
};
} // namespace Galaxy
