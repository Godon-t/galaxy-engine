#pragma once

#include "commands/RenderCommand.hpp"
#include "core/Log.hpp"
#include "rendering/GPUInstances/DebugLines.hpp"
#include "rendering/GPUInstances/FrameBuffer.hpp"
#include "rendering/GPUInstances/MaterialInstance.hpp"
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
struct RenderGraphExecution;
class Renderer;

class Backend {
public:
    Backend(size_t maxSize = 512);
    ~Backend();

    BufferHandle instantiateUBO(unsigned int dataSize);

    ProgramHandle loadShader(std::string path);
    void clearProgram(ProgramHandle program);

    GeometryHandle instanciateMesh(ResourceHandle<Mesh> mesh, int surfaceIdx);
    GeometryHandle instanciateMesh(std::vector<Vertex>& vertices, std::vector<unsigned short>& indices, std::function<void()> destroyCallback = nullptr);
    Sphere getMeshBoundingVolume(GeometryHandle mesh);
    void clearMesh(GeometryHandle mesh);

    TextureHandle instantiateTexture(TextureFormat format, vec2 size, TextureFiltering filter = TextureFiltering::LINEAR, size_t layerCount = 0);
    TextureHandle instantiateTexture(ResourceHandle<Image> image);
    void setTextureWrap(TextureHandle texture, TextureWrap wrapS, TextureWrap wrapT);
    void clearTexture(TextureHandle texture);
    
    
    void frameReset();
    [[nodiscard]] std::size_t getDrawCallsCount() const noexcept { return m_drawCount; }

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
    void resizeFrameBuffer(FramebufferHandle framebuffer, unsigned int width, unsigned int height, int depthLayerCount = -1);
    void resizeCubemapFrameBuffer(CubemapFramebufferHandle framebuffer, unsigned int size);
    // TODO: Wrong way ?
    FramebufferTextureFormat getFramebufferFormat(FramebufferHandle framebuffer);

    void setProjectionMatrix(const mat4& projectionMatrix);
    unsigned int getFrameBufferTextureID(FramebufferHandle framebuffer);
    unsigned int getFrameBufferDepthTextureID(FramebufferHandle framebuffer);

    void setCullMode(GeometryHandle geometry, CullMode mode);

    [[nodiscard]] bool isValid(ProgramHandle handle) const noexcept { return m_programInstances.contains(handle); }
    [[nodiscard]] bool isValid(GeometryHandle handle) const noexcept { return m_visualInstances.contains(handle); }
    [[nodiscard]] bool isValid(TextureHandle handle) const noexcept { return m_textureInstances.contains(handle); }
    [[nodiscard]] bool isValid(MaterialHandle handle) const noexcept { return m_materialInstances.contains(handle); }
    [[nodiscard]] bool isValid(CubemapHandle handle) const noexcept { return m_cubemapInstances.contains(handle); }
    [[nodiscard]] bool isValid(FramebufferHandle handle) const noexcept { return m_frameBufferInstances.contains(handle); }
    [[nodiscard]] bool isValid(CubemapFramebufferHandle handle) const noexcept { return m_cubemapFrameBufferInstances.contains(handle); }
    [[nodiscard]] bool isValid(BufferHandle handle) const noexcept { return m_uboInstances.contains(handle); }

    void initDebugCallback();

    void destroy();
    void setActiveProgram(ProgramHandle program);
    [[nodiscard]] int getUniformLocation(ProgramHandle program, const std::string& uniformName) const;
    
    
    void bindFramebuffer(FramebufferHandle fb, int targetLayer = -1);
    void unbindFramebuffer(FramebufferHandle fb);
    bool attachColorTextureToFramebuffer(TextureHandle texture, FramebufferHandle framebuffer, int colorattachmentIdx);
    bool attachDepthTextureToFramebuffer(TextureHandle texture, FramebufferHandle framebuffer);
    
    void execute(RenderGraphExecution& execution);
    
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

    void draw(const GeometryHandle geometryH, const mat4 model);
    void draw(const GeometryHandle geometryH);
    void attachCubemapToFramebuffer(const CubemapFramebufferHandle fbHandle, const CubemapHandle cubemapHandle, const size_t colorIdx);
    void bindMaterial(const MaterialHandle handle);
    void processCommand(const SetUniformCommand& command);
    void processCommand(const UpdateTextureCommand& command);
    void processCommand(const UpdateUBOCommand& command);
    void processCommand(const BindUBOCommand& command);

    void processCommand(const SaveFrameBufferCommand& command);

    Program* getActiveProgram();
    const Program* getActiveProgram() const;

    GpuResourceRegistry<Program, ProgramHandle> m_programInstances;
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

    ProgramHandle m_activeProgram;
    std::size_t m_drawCount = 0;

    friend class Renderer;
};
} // namespace Galaxy
