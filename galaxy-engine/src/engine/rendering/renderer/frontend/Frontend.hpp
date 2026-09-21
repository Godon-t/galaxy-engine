#pragma once

#include "pch.hpp"

#include "SceneContext.hpp"
#include "rendering/renderer/commands/RenderCommand.hpp"
#include "data/Transform.hpp"
#include "types/Render.hpp"
#include "RenderDevice.hpp"
#include "common/geometry/Shapes.hpp"
#include "engine/rendering/renderer/refactor/RenderGraphDeclaration.hpp"
#include "engine/rendering/renderer/refactor/RenderGraphCompilation.hpp"
#include "engine/rendering/renderer/refactor/RenderGraph.hpp"
#include "engine/rendering/renderer/refactor/RenderGraphExecution.hpp"
#include "engine/rendering/renderer/LightManager.hpp"

#include "queue"
#include <memory>
#include <optional>

namespace Galaxy {
class Backend;

class Frontend {
public:
    void addRenderDevice(std::unique_ptr<RenderDevice> renderDevice){ m_frameDevices.push_back(std::move(renderDevice)); }

    explicit Frontend(Backend& backend);

    // void storeCanvaResult(std::string& path);

    void submit(GeometryHandle geometry);
    void submit(GeometryHandle geometry, const Transform& transform);
    void clear(vec4& color);

    // void bindTexture(TextureHandle texture, std::string uniformName, bool important = false);
    // void attachTextureToColorFramebuffer(TextureHandle texture, FramebufferHandle framebuffer, int attachmentIdx);
    // void attachTextureToDepthFramebuffer(TextureHandle texture, FramebufferHandle framebuffer);
    // void attachCubemapToFramebuffer(CubemapHandle cubemap, CubemapFramebufferHandle framebuffer, int colorIdx = 0);
    // void useCubemap(CubemapHandle cubemap, std::string uniformName);
    // void bindFrameBuffer(FramebufferHandle framebuffer, int depthLayerIdx = -1);
    // void bindCubemapFrameBuffer(CubemapFramebufferHandle framebuffer, int cubemapFaceIdx);
    // void unbindFrameBuffer(FramebufferHandle framebuffer);
    // void unbindCubemapFrameBuffer(CubemapFramebufferHandle framebuffer);
    // void bindMaterial(MaterialHandle material);
    // TODO: rename to match setActiveProgram command
    void changeUsedProgram(ProgramType program);
    void changeUsedProgram(ProgramHandle program);

    // void setUniform(std::string uniformName, bool value);
    // void setUniform(std::string uniformName, float value);
    // void setUniform(std::string uniformName, int value);
    // void setUniform(std::string uniformName, mat4 value);
    // void setUniform(std::string uniformName, vec3 value);
    // void setUniform(std::string uniformName, ivec3 value);
    // void setUniform(std::string uniformName, vec2 value);
    // template <typename T>
    // void updateUniform(BufferHandle ubo, const T& payload)
    // {
    //     auto updateCommand = UpdateUBOCommand::make(ubo, payload);
    //     pushCommand(std::move(updateCommand));
    // }
    // void bindUBO(BufferHandle ubo, unsigned int idx);

    // void setFramebufferAsTextureUniform(FramebufferHandle framebuffer, std::string uniformName, int textureIdx);
    // void setFramebufferAsCubemapUniform(CubemapFramebufferHandle framebuffer, std::string uniformName, int colorIdx);

    // void setViewport(vec2 position, vec2 dimmension);
    // void resizeTexture(TextureHandle texture, unsigned int width, unsigned int height);
    // void setTextureFormat(TextureHandle texture, TextureFormat format);
    // void updateCubemap(CubemapHandle cubemap, unsigned int resolution);

    void addDebugMsg(std::string message);
    void submitDebugLine(vec3 start, vec3 end, vec3 color);
    void drawDebug();

    void addObjectToScene(GeometryHandle geometry, const Sphere& boundingVolume, std::optional<MaterialHandle> material, const Transform& transform);

    inline void removeMaterialID(MaterialHandle material) { m_frameContext.removeMaterial(material); }
    void notifyMaterialUpdated(MaterialHandle material, bool isTransparent);
    inline void clearContext(){m_frameContext.clear();}

    RenderGraphExecution buildFrameExecution();

    FramebufferHandle getFinalFramebuffer();
    LightManager& getLightManager() { return m_lightManager; }
    
    
private:
    // TODO: Create cameraFrustum object
    // void setViewMatrix(const math::mat4& view);
    // void setProjectionMatrix(const math::mat4& projection);
    // void pushCommand(RenderCommand command);
    void saveFrameBuffer(FramebufferHandle framebuffer, std::string path);

    bool m_addCommandsToDevice = false;

    GeometryHandle m_postProcessingQuad;

    LightManager m_lightManager;
    SceneContext m_frameContext;
    std::vector<std::unique_ptr<RenderDevice>> m_frameDevices;

    FramebufferTextureFormat m_currentFramebufferFormat = FramebufferTextureFormat::None;

    RenderGraph m_renderGraph;

    RenderPassId m_passShadowId;
    RenderPassId m_passOpaquePBRId;
    RenderPassId m_passTransparentPBRId;
    RenderPassId m_passTextureId;
    RenderPassId m_passUnicolorId;
    RenderPassId m_passSkyboxId;
    RenderPassId m_passPostprocessId;
};
} // namespace Galaxy
