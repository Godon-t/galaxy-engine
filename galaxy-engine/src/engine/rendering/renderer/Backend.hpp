#pragma once

#include "commands/RenderCommand.hpp"
#include "core/Log.hpp"
#include "rendering/GPUInstances/DebugLines.hpp"
#include "rendering/GPUInstances/FrameBuffer.hpp"
#include "rendering/GPUInstances/Texture.hpp"
#include "rendering/GPUInstances/UBOInstance.hpp"
#include "rendering/GPUInstances/VisualInstance.hpp"
#include "rendering/Program.hpp"
#include "resources/GpuResourceRegistry.hpp"
#include "resource/Image.hpp"
#include "resource/Mesh.hpp"
#include "resource/ResourceHandle.hpp"
#include "types/Render.hpp"
#include <functional>
#include <memory>

namespace Galaxy {
class Renderer;

class Backend {
public:
    Backend(size_t maxSize = 512);
    ~Backend();

    BufferHandle instantiateUBO(unsigned int dataSize);

    GeometryHandle instanciateMesh(ResourceHandle<Mesh> mesh, int surfaceIdx);
    GeometryHandle instanciateMesh(std::vector<Vertex>& vertices, std::vector<unsigned short>& indices, std::function<void()> destroyCallback = nullptr);
    Sphere getMeshBoundingVolume(GeometryHandle mesh);
    void clearMesh(GeometryHandle mesh);

    TextureHandle instantiateTexture(TextureFormat format, vec2 size);
    TextureHandle instantiateTexture(ResourceHandle<Image> image);
    void clearTexture(TextureHandle texture);
    void frameReset();

    MaterialHandle instanciateMaterial(ResourceHandle<Material> material);
    void updateMaterial(MaterialHandle materialHandle, ResourceHandle<Material> material);
    void clearMaterial(MaterialHandle material);

    using MaterialUpdateCallback = std::function<void(MaterialHandle material, bool isTransparent)>;
    void onMaterialUpdated(MaterialUpdateCallback callback) { m_materialUpdateCallback = callback; }

    void processCommands(const std::vector<RenderCommand>& commands);

    GeometryHandle generateCube(float dimmension, bool inward, std::function<void()> destroyCallback);
    GeometryHandle generateQuad(vec2 dimmensions, std::function<void()> destroyCallback);
    GeometryHandle generatePyramid(float baseSize, float height, std::function<void()> destroyCallback);

    CubemapHandle instanciateCubemap(std::array<ResourceHandle<Image>, 6> faces);
    CubemapHandle instanciateCubemap(int resolution = 1024);
    void clearCubemap(CubemapHandle cubemap);

    FramebufferHandle instanciateFrameBuffer(unsigned int width, unsigned int height, FramebufferTextureFormat format, unsigned int colorCount = 1, unsigned int depthLayerCount = 0);
    CubemapFramebufferHandle instantiateCubemapFrameBuffer(unsigned int resolution, unsigned int colorCount = 1);

    void clearFrameBuffer(FramebufferHandle framebuffer);
    void clearFrameBuffer(CubemapFramebufferHandle framebuffer);
    void resizeFrameBuffer(FramebufferHandle framebuffer, unsigned int width, unsigned int height, unsigned int depthLayerCount = 0);
    void resizeCubemapFrameBuffer(CubemapFramebufferHandle framebuffer, unsigned int size);
    // TODO: Wrong way ?
    FramebufferTextureFormat getFramebufferFormat(FramebufferHandle framebuffer);

    void setProjectionMatrix(const mat4& projectionMatrix);
    unsigned int getFrameBufferTextureID(FramebufferHandle framebuffer);
    unsigned int getFrameBufferDepthTextureID(FramebufferHandle framebuffer);

    void setCullMode(GeometryHandle geometry, CullMode mode);

    [[nodiscard]] bool isValid(GeometryHandle handle) const noexcept { return m_visualInstances.contains(handle); }
    [[nodiscard]] bool isValid(TextureHandle handle) const noexcept { return m_textureInstances.contains(handle); }
    [[nodiscard]] bool isValid(MaterialHandle handle) const noexcept { return m_materialInstances.contains(handle); }
    [[nodiscard]] bool isValid(CubemapHandle handle) const noexcept { return m_cubemapInstances.contains(handle); }
    [[nodiscard]] bool isValid(FramebufferHandle handle) const noexcept { return m_frameBufferInstances.contains(handle); }
    [[nodiscard]] bool isValid(CubemapFramebufferHandle handle) const noexcept { return m_cubemapFrameBufferInstances.contains(handle); }
    [[nodiscard]] bool isValid(BufferHandle handle) const noexcept { return m_uboInstances.contains(handle); }

    void initDebugCallback();

    void destroy();
    void setActiveProgram(ProgramType program);

private:
    struct FramebufferAttachments {
        std::vector<TextureHandle> colors;
        TextureHandle depth;
    };

    struct CubemapFramebufferAttachments {
        std::vector<CubemapHandle> colors;
        CubemapHandle depth;
    };

    void releaseAttachments(FramebufferHandle framebuffer);
    void releaseAttachments(CubemapFramebufferHandle framebuffer);

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

    GpuResourceRegistry<VisualInstance, GeometryHandle> m_visualInstances;
    GpuResourceRegistry<Texture, TextureHandle> m_textureInstances;
    GpuResourceRegistry<MaterialInstance, MaterialHandle> m_materialInstances;
    GpuResourceRegistry<Cubemap, CubemapHandle> m_cubemapInstances;
    GpuResourceRegistry<FrameBuffer, FramebufferHandle> m_frameBufferInstances;
    GpuResourceRegistry<CubemapFrameBuffer, CubemapFramebufferHandle> m_cubemapFrameBufferInstances;
    GpuResourceRegistry<UBOInstance, BufferHandle> m_uboInstances;

    std::unordered_map<GeometryHandle, std::function<void()>, GpuResourceHandleHash> m_visualDestroyNotifications;
    std::unordered_map<TextureHandle, std::function<void()>, GpuResourceHandleHash> m_textureDestroyNotifications;
    std::unordered_map<MaterialHandle, std::function<void()>, GpuResourceHandleHash> m_materialDestroyNotifications;
    std::unordered_map<CubemapHandle, std::shared_ptr<int>, GpuResourceHandleHash> m_cubemapUploadLifetimes;
    std::unordered_map<FramebufferHandle, FramebufferAttachments, GpuResourceHandleHash> m_framebufferAttachments;
    std::unordered_map<CubemapFramebufferHandle, CubemapFramebufferAttachments, GpuResourceHandleHash> m_cubemapFramebufferAttachments;
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
