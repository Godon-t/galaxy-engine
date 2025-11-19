#pragma once

#include "RenderCommand.hpp"
#include "core/Log.hpp"
#include "rendering/GPUInstances/FrameBuffer.hpp"
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

    renderID instantiateTexture();
    renderID instantiateTexture(ResourceHandle<Image> image);
    void clearTexture(renderID textureID);

    renderID instanciateMaterial(ResourceHandle<Material> material);
    void updateMaterial(renderID materialID, ResourceHandle<Material> material);
    void clearMaterial(renderID materialID);

    void processCommands(std::vector<RenderCommand>& commands);

    renderID generateCube(float dimmension, bool inward, std::function<void()> destroyCallback);
    renderID generateQuad(vec2 dimmensions, std::function<void()> destroyCallback);
    renderID generatePyramid(float baseSize, float height, std::function<void()> destroyCallback);

    renderID instanciateCubemap(std::array<ResourceHandle<Image>, 6> faces);
    renderID instanciateCubemap(int resolution = 1024);
    void clearCubemap(renderID cubemapID);

    renderID instanciateFrameBuffer(unsigned int width, unsigned int height, FramebufferTextureFormat format);
    renderID instantiateCubemapFrameBuffer(unsigned int size);
    
    void clearFrameBuffer(renderID frameBufferID);
    void resizeFrameBuffer(renderID frameBufferID, unsigned int width, unsigned int height);
    void resizeCubemapFrameBuffer(renderID frameBufferID, unsigned int size);
    // TODO: Wrong way ?
    FramebufferTextureFormat getFramebufferFormat(renderID id);

    void setProjectionMatrix(const mat4& projectionMatrix);
    unsigned int getFrameBufferTextureID(renderID frameBufferID);
    unsigned int getFrameBufferDepthTextureID(renderID frameBufferID);

    void destroy();

private:
    void processCommand(RenderCommand& command);
    void processCommand(ClearCommand& command);
    void processCommand(DepthMaskCommand& command);
    void processCommand(SetViewCommand& command);
    void processCommand(SetProjectionCommand& command);
    void processCommand(SetActiveProgramCommand& command);
    void processCommand(DrawCommand& command);
    void processCommand(RawDrawCommand& command);
    void processCommand(UseTextureCommand& command);
    void processCommand(UseCubemapCommand& command);
    void processCommand(AttachTextureToFramebufferCommand& command);
    void processCommand(AttachCubemapToFramebufferCommand& command);
    void processCommand(BindMaterialCommand& command);
    void processCommand(BindFrameBufferCommand& command, bool bind);
    void processCommand(InitPostProcessCommand& command);
    void processCommand(SetUniformCommand& command);
    void processCommand(SetViewportCommand& command);
    void processCommand(UpdateCubemapCommand& command);

    void processCommand(DebugMsgCommand& command);

    RenderGpuResourceTable<VisualInstance> m_visualInstances;
    RenderGpuResourceTable<Texture> m_textureInstances;
    RenderGpuResourceTable<MaterialInstance> m_materialInstances;
    RenderGpuResourceTable<Cubemap> m_cubemapInstances;
    RenderGpuResourceTable<FrameBuffer> m_frameBufferInstances;
    RenderGpuResourceTable<CubemapFrameBuffer> m_cubemapFrameBufferInstances;

    // Will invalidate renderID for things outside of Node that store a renderID
    std::unordered_map<renderID, std::function<void()>> m_gpuDestroyNotifications;

    ProgramPBR m_mainProgram;
    ProgramSkybox m_skyboxProgram;
    ProgramSkybox m_irradianceProgram;
    ProgramTexture m_textureProgram;
    ProgramUnicolor m_unicolorProgram;
    ProgramPostProc m_postProcessingProgram;
    ProgramShadow m_shadowProgram;
    Program* m_activeProgram;

    friend class Renderer;
};
} // namespace Galaxy
