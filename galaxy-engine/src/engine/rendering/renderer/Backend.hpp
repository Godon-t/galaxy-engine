#pragma once

#include "RenderCommand.hpp"
#include "core/Log.hpp"
#include "rendering/GPUInstances/DebugLines.hpp"
#include "rendering/GPUInstances/FrameBuffer.hpp"
#include "rendering/GPUInstances/Texture.hpp"
#include "rendering/GPUInstances/UBOInstance.hpp"
#include "rendering/GPUInstances/VisualInstance.hpp"
#include "rendering/Program.hpp"
#include "resource/Image.hpp"
#include "resource/Mesh.hpp"
#include "resource/ResourceHandle.hpp"
#include "types/Render.hpp"
#include <functional>
#include <memory>
#include <tuple>
#include <utility>

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
        return tryRemove(idToRemove, [](T&) {});
    }

    template <typename BeforeRemove>
    bool tryRemove(renderID idToRemove, BeforeRemove&& beforeRemove)
    {
        auto instance = m_renderIdToInstance.find(idToRemove);
        if (instance == m_renderIdToInstance.end()) {
            GLX_CORE_ERROR("Trying to remove an unknown GPU resource: {0}", idToRemove);
            return false;
        }

        if (instance->second.second > 1) {
            --instance->second.second;
            return false;
        }

        beforeRemove(instance->second.first);
        m_renderIdToInstance.erase(instance);
        m_freeIds.emplace(idToRemove);
        return true;
    }

    template <typename... Args>
    renderID createResourceInstance(Args&&... args)
    {
        if (m_freeIds.size() == 0) {
            GLX_CORE_ERROR("No more free renderIDs");
            return 0;
        }

        renderID createdID = m_freeIds.top();

        const auto insertion = m_renderIdToInstance.try_emplace(
            createdID,
            std::piecewise_construct,
            std::forward_as_tuple(std::forward<Args>(args)...),
            std::forward_as_tuple(size_t { 1 }));
        const bool inserted = insertion.second;
        GLX_CORE_ASSERT(inserted, "Duplicate GPU resource ID: {0}", createdID);
        if (!inserted)
            return 0;

        m_freeIds.pop();

        return createdID;
    }

    void increaseCount(renderID id)
    {
        auto instance = m_renderIdToInstance.find(id);
        GLX_CORE_ASSERT(instance != m_renderIdToInstance.end(), "Unknown GPU resource: {0}", id);
        if (instance != m_renderIdToInstance.end())
            ++instance->second.second;
    }

    T* get(renderID id)
    {
        auto instance = m_renderIdToInstance.find(id);
        GLX_CORE_ASSERT(instance != m_renderIdToInstance.end(), "Unknown GPU resource: {0}", id);
        return instance != m_renderIdToInstance.end() ? &instance->second.first : nullptr;
    }

    std::vector<T*> getAll(){
        std::vector<T*> res;
        res.reserve(m_renderIdToInstance.size());
        
        for(auto& [id, value] : m_renderIdToInstance){
            res.push_back(&value.first);
        }
        return res;
    }

    bool canAddInstance() { return m_freeIds.size() > 0; }
    void removeAll()
    {
        for (auto& elem : m_renderIdToInstance) {
            m_freeIds.emplace(elem.first);
        }
        m_renderIdToInstance.clear();
    }

    T* tryGet(renderID id) noexcept
    {
        auto instance = m_renderIdToInstance.find(id);
        return instance != m_renderIdToInstance.end() ? &instance->second.first : nullptr;
    }

private:
    std::unordered_map<renderID, std::pair<T, size_t>> m_renderIdToInstance;
    std::stack<renderID> m_freeIds;
};

class Backend {
public:
    Backend(size_t maxSize = 512);
    ~Backend();

    renderID instantiateUBO(unsigned int dataSize);

    renderID instanciateMesh(ResourceHandle<Mesh> mesh, int surfaceIdx);
    renderID instanciateMesh(std::vector<Vertex>& vertices, std::vector<unsigned short>& indices, std::function<void()> destroyCallback = nullptr);
    Sphere& getMeshBoundingVolume(renderID meshID);
    void clearMesh(renderID meshID);

    renderID instantiateTexture(TextureFormat format, vec2 size);
    renderID instantiateTexture(ResourceHandle<Image> image);
    void clearTexture(renderID textureID);
    void frameReset();

    renderID instanciateMaterial(ResourceHandle<Material> material);
    void updateMaterial(renderID materialID, ResourceHandle<Material> material);
    void clearMaterial(renderID materialID);

    using MaterialUpdateCallback = std::function<void(renderID materialID, bool isTransparent)>;
    void onMaterialUpdated(MaterialUpdateCallback callback) { m_materialUpdateCallback = callback; }

    void processCommands(const std::vector<RenderCommand>& commands);

    renderID generateCube(float dimmension, bool inward, std::function<void()> destroyCallback);
    renderID generateQuad(vec2 dimmensions, std::function<void()> destroyCallback);
    renderID generatePyramid(float baseSize, float height, std::function<void()> destroyCallback);

    renderID instanciateCubemap(std::array<ResourceHandle<Image>, 6> faces);
    renderID instanciateCubemap(int resolution = 1024);
    void clearCubemap(renderID cubemapID);

    renderID instanciateFrameBuffer(unsigned int width, unsigned int height, FramebufferTextureFormat format, unsigned int colorCount = 1, unsigned int depthLayerCount = 0);
    renderID instantiateCubemapFrameBuffer(unsigned int resolution, unsigned int colorCount = 1);

    void clearFrameBuffer(renderID frameBufferID);
    void resizeFrameBuffer(renderID frameBufferID, unsigned int width, unsigned int height, unsigned int depthLayerCount = 0);
    void resizeCubemapFrameBuffer(renderID frameBufferID, unsigned int size);
    // TODO: Wrong way ?
    FramebufferTextureFormat getFramebufferFormat(renderID id);

    void setProjectionMatrix(const mat4& projectionMatrix);
    unsigned int getFrameBufferTextureID(renderID frameBufferID);
    unsigned int getFrameBufferDepthTextureID(renderID frameBufferID);

    void setCullMode(renderID visualInstanceID, CullMode mode);

    void initDebugCallback();

    void destroy();
    void setActiveProgram(ProgramType program);

private:
    void processCommand(const ClearCommand& command);
    void processCommand(const DepthMaskCommand& command);
    void processCommand(const SetViewCommand& command);
    void processCommand(const SetProjectionCommand& command);
    void processCommand(const SetActiveProgramCommand& command);
    void processCommand(const DrawCommand& command);
    void processCommand(const RawDrawCommand& command);
    void processCommand(const UseTextureCommand& command);
    void processCommand(const UseCubemapCommand& command);
    void processCommand(const AttachTextureToFramebufferCommand& command);
    void processCommand(const AttachCubemapToFramebufferCommand& command);
    void processCommand(const BindMaterialCommand& command);
    void processCommand(const BindFrameBufferCommand& command);
    void processCommand(const SetUniformCommand& command);
    void processCommand(const SetViewportCommand& command);
    void processCommand(const UpdateTextureCommand& command);
    void processCommand(const UpdateCubemapCommand& command);
    void processCommand(const SetFramebufferAsTextureUniformCommand& command);
    void processCommand(const UpdateUBOCommand& command);
    void processCommand(const BindUBOCommand& command);

    void processCommand(const DebugMsgCommand& command);
    void processCommand(const DrawDebugLineCommand& command);
    void processCommand(const SaveFrameBufferCommand& command);
    void debugDraw();

    RenderGpuResourceTable<VisualInstance> m_visualInstances;
    RenderGpuResourceTable<Texture> m_textureInstances;
    RenderGpuResourceTable<MaterialInstance> m_materialInstances;
    RenderGpuResourceTable<Cubemap> m_cubemapInstances;
    RenderGpuResourceTable<FrameBuffer> m_frameBufferInstances;
    RenderGpuResourceTable<CubemapFrameBuffer> m_cubemapFrameBufferInstances;
    RenderGpuResourceTable<UBOInstance> m_uboInstances;

    // Temporary until renderID becomes a typed generational handle.
    std::unordered_map<renderID, std::function<void()>> m_visualDestroyNotifications;
    std::unordered_map<renderID, std::function<void()>> m_textureDestroyNotifications;
    std::unordered_map<renderID, std::function<void()>> m_materialDestroyNotifications;
    std::unordered_map<renderID, std::shared_ptr<int>> m_cubemapUploadLifetimes;
    std::shared_ptr<int> m_lifetimeToken = std::make_shared<int>(0);

    MaterialUpdateCallback m_materialUpdateCallback;

    ProgramPBR m_mainProgram;
    ProgramSkybox m_skyboxProgram;
    ProgramSkybox m_irradianceProgram;
    ProgramTexture m_textureProgram;
    ProgramUnicolor m_unicolorProgram;
    ProgramPostProc m_postProcessingProbeProgram;
    ProgramPostProcSSGI m_postProcessingSSGIProgram;
    ProgramShadow m_shadowProgram;
    ProgramComputeOctahedral m_computeOctahedralProgram;
    Program* m_activeProgram;

    ProgramDebugLines m_debugLinesProgram;
    DebugLines m_debugLines;

    friend class Renderer;
};
} // namespace Galaxy
