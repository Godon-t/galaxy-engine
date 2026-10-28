#pragma once

#include "pch.hpp"

#include "resource/Environment.hpp"
#include "SceneContext.hpp"
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
    void setEnvironment(ResourceHandle<Environment> environment);

    explicit Frontend(Backend& backend);

    void addObjectToScene(GeometryHandle geometry, std::optional<MaterialHandle> material,
        const Transform& transform = Transform {}, const BitMask& layer = BitMask {});
    void addObjectToScene(GeometryHandle geometry, const Sphere& boundingVolume, std::optional<MaterialHandle> material,
        const Transform& transform = Transform {}, const BitMask& layer = BitMask {});

    inline void removeMaterialID(MaterialHandle material) { m_frameContext.removeMaterial(material); }
    void notifyMaterialUpdated(MaterialHandle material, const MaterialRenderInfo& info);
    inline void clearContext(){m_frameContext.clear();}

    RenderGraphExecution buildFrameExecution();

    FramebufferHandle getFinalFramebuffer();
    LightManager& getLightManager() { return m_lightManager; }
    
    
private:
    Backend& m_backend;

    void addShadowViews(RenderGraphExecution& execution);
    void addPointViews(RenderGraphExecution& execution, RenderPoint& device);
    void addCameraView(RenderGraphExecution& execution, RenderCamera& device);
    void addTextureViews(RenderGraphExecution& execution, const RenderDevice& device,
        const mat4& projection, const mat4& view, const std::vector<RenderItem>& items,
        bool transparent, std::optional<RenderTargetHandle> targetFramebuffer, int targetFace);
    void addSceneView(RenderGraphExecution& execution, const RenderDevice& device,
        const mat4& projection, const mat4& view, const vec3& position,
        std::vector<RenderItem> opaques, std::vector<RenderItem> transparents,
        std::optional<RenderTargetHandle> targetFramebuffer = std::nullopt, int targetFace = -1);
    static void configureView(RenderPassInvocation& invocation, const RenderDevice& device,
        const mat4& projection, const mat4& view);

    GeometryHandle m_postProcessingQuad;
    GeometryHandle m_skyboxCube;

    LightManager m_lightManager;
    SceneContext m_frameContext;
    std::vector<std::unique_ptr<RenderDevice>> m_frameDevices;

    RenderGraph m_renderGraph;

    RenderPassId m_passShadowId;
    RenderPassId m_passOpaquePBRId;
    RenderPassId m_passTransparentPBRId;
    RenderPassId m_passTextureId;
    RenderPassId m_passTransparentTextureId;
    RenderPassId m_passUnicolorId;
    RenderPassId m_passLightingId;
    RenderPassId m_passPostprocessId;
    RenderPassId m_passCubemapRenderId;
    RenderPassId m_passTransparentCubemapId;
};
} // namespace Galaxy
