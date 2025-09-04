#pragma once

#include "RenderCommand.hpp"
#include "core/Log.hpp"
#include "rendering/GPUInstances/Texture.hpp"
#include "rendering/GPUInstances/VisualInstance.hpp"
#include "rendering/Program.hpp"
#include "resource/Image.hpp"
#include "resource/Mesh.hpp"
#include "resource/ResourceHandle.hpp"
#include "types/Render.hpp"

namespace Galaxy {

template <typename T>
class RenderGpuResourceTable {
public:
    RenderGpuResourceTable(int maxSize = 512)
    {
        m_renderIdToInstance.reserve(maxSize);
        for (size_t i = 0; i < maxSize; i++) {
            m_freeIds.push(i);
        }
    }

    bool tryRemove(renderID idToRemove)
    {
        m_renderIdToInstance[idToRemove].second--;
        if (m_renderIdToInstance[idToRemove].second > 0)
            return false;

        m_renderIdToInstance.erase(idToRemove);

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

        m_renderIdToInstance[createdID] = std::make_pair(T(), 1);

        m_freeIds.pop();

        return createdID;
    }

    void increaseCount(renderID id)
    {
        m_renderIdToInstance[id].second++;
    }

    T* get(renderID id)
    {
        return &m_renderIdToInstance[id].first;
    }

    bool canAddInstance() { return m_freeIds.size() > 0; }

private:
    std::unordered_map<renderID, std::pair<T, size_t>> m_renderIdToInstance;
    std::stack<renderID> m_freeIds;
};

class Backend {
public:
    Backend(size_t maxSize = 512);

    renderID instanciateMesh(ResourceHandle<Mesh> mesh, int surfaceIdx);
    renderID instanciateMesh(std::vector<Vertex>& vertices, std::vector<unsigned short>& indices);
    void clearMesh(renderID meshID);

    renderID instanciateTexture(ResourceHandle<Image> image);

    void processCommands(std::vector<RenderCommand>& commands);

private:
    struct MeshHandle {
        ResourceHandle<Mesh> mesh;
        std::unordered_map<int, renderID> activesSubMeshes;
    };

    struct ImageHandle {
        ResourceHandle<Image> image;
        renderID textureID;
    };

    void processCommand(RenderCommand& command);

    std::unordered_map<size_t, MeshHandle> m_meshResourceTable;
    std::unordered_map<renderID, size_t> m_idToResource;
    RenderGpuResourceTable<VisualInstance> m_visualInstances;

    std::unordered_map<size_t, ImageHandle> m_imageResourceTable;
    std::unordered_map<renderID, size_t> m_idToImageResource;
    RenderGpuResourceTable<Texture> m_textureInstances;

    Program m_mainProgram;
};
} // namespace Galaxy
