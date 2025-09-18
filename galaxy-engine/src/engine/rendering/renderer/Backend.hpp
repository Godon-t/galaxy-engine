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
class Renderer;

template <typename T>
class RenderGpuResourceTable {
public:
    RenderGpuResourceTable(int maxSize = 512)
    {
        m_renderIdToInstance.reserve(maxSize);
        // 0 reserved for invalid renderID
        for (size_t i = 1; i <= maxSize; i++) {
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

        m_renderIdToInstance.insert_or_assign(createdID, std::make_pair(T(), 1));

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
    void removeAll(std::function<void(T&)> deletionCallback)
    {
        for (auto& elem : m_renderIdToInstance) {
            deletionCallback(elem.second.first);
            m_freeIds.emplace(elem.first);
        }
        m_renderIdToInstance.clear();
    }

private:
    std::unordered_map<renderID, std::pair<T, size_t>> m_renderIdToInstance;
    std::stack<renderID> m_freeIds;
};

class Backend {
public:
    Backend(size_t maxSize = 512);

    renderID instanciateMesh(ResourceHandle<Mesh> mesh, int surfaceIdx);
    renderID instanciateMesh(std::vector<Vertex>& vertices, std::vector<unsigned short>& indices, std::function<void()> destroyCallback = nullptr);
    void clearMesh(renderID meshID);

    renderID instanciateTexture(ResourceHandle<Image> image);
    void clearTexture(renderID textureID);

    renderID instanciateMaterial(ResourceHandle<Material> material);
    void updateMaterial(renderID materialID, ResourceHandle<Material> material);
    void clearMaterial(renderID materialID);

    void processCommands(std::vector<RenderCommand>& commands);

    renderID generateCube(float dimmension, bool inward, std::function<void()> destroyCallback);
    renderID instanciateCubemap(std::array<ResourceHandle<Image>, 6> faces);
    renderID instanciateCubemap();
    void clearCubemap(renderID cubemapID);

    // TODO: I don't think it is a good idea
    inline mat4 getProjectionMatrix() { return m_projectionMatrix; }
    void setProjectionMatrix(const mat4& projectionMatrix);

    void destroy();

private:
    void processCommand(RenderCommand& command);
    void processCommand(ClearCommand& command);
    void processCommand(DepthMaskCommand& command);
    void processCommand(SetViewCommand& command);
    void processCommand(SetProjectionCommand& command);
    void processCommand(SetActiveProgramCommand& command);
    void processCommand(DrawCommand& command);
    void processCommand(BindTextureCommand& command);
    void processCommand(BindCubemapCommand& command);
    void processCommand(BindMaterialCommand& command);

    RenderGpuResourceTable<VisualInstance> m_visualInstances;
    RenderGpuResourceTable<Texture> m_textureInstances;
    RenderGpuResourceTable<MaterialInstance> m_materialInstances;
    RenderGpuResourceTable<Cubemap> m_cubemapInstances;

    // Will invalidate renderID for things outside of Node that store a renderID
    std::unordered_map<renderID, std::function<void()>> m_gpuDestroyNotifications;

    ProgramPBR m_mainProgram;
    ProgramSkybox m_skyboxProgram;
    ProgramTexture m_textureProgram;
    Program* m_activeProgram;

    mat4 m_projectionMatrix;

    friend class Renderer;
};
} // namespace Galaxy
