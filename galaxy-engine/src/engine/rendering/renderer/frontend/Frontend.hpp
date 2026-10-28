#pragma once

#include "pch.hpp"

#include "resource/Environment.hpp"
#include "SceneContext.hpp"
#include "data/Transform.hpp"
#include "types/Render.hpp"
#include "data/Camera.hpp"
#include "common/geometry/Shapes.hpp"
#include "engine/rendering/renderer/RenderGraph/RenderGraphDeclaration.hpp"
#include "engine/rendering/renderer/RenderGraph/RenderGraphCompilation.hpp"
#include "engine/rendering/renderer/RenderGraph/RenderGraph.hpp"
#include "engine/rendering/renderer/RenderGraph/RenderGraphExecution.hpp"
#include "engine/rendering/renderer/LightManager.hpp"

#include <optional>
#include <utility>
#include <vector>

namespace Galaxy {

class Backend;

class Frontend {
public:
    void addCamera(Camera camera) { m_frameCameras.push_back(std::move(camera)); }
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
    void addTextureViews(RenderGraphExecution& execution, const RenderView& view,
        const std::vector<RenderItem>& items, bool transparent);
    void addSceneView(RenderGraphExecution& execution, const RenderView& view);
    static void configureView(RenderPassInvocation& invocation, const RenderView& view);

    GeometryHandle m_postProcessingQuad;
    GeometryHandle m_skyboxCube;

    LightManager m_lightManager;
    SceneContext m_frameContext;
    std::vector<Camera> m_frameCameras;

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
