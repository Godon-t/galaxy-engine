#pragma once

#include "pch.hpp"

#include "resource/EnvironmentMap.hpp"
#include "SceneContext.hpp"
#include "data/Transform.hpp"
#include "types/Render.hpp"
#include "data/Camera.hpp"
#include "common/geometry/Shapes.hpp"
#include "engine/rendering/RenderGraph/RenderGraphDeclaration.hpp"
#include "engine/rendering/RenderGraph/RenderGraphCompilation.hpp"
#include "engine/rendering/RenderGraph/RenderGraph.hpp"
#include "engine/rendering/frontend/LightManager.hpp"

#include <memory>
#include <utility>

namespace Galaxy {

class Backend;

class Frontend {
public:
    void addCamera(Camera camera) { m_frameCameras.push_back(std::move(camera)); }
    GraphResourceIdTexture getEnvironmentGraphTexture();
    GraphResourceIdTexture getIrradianceGraphTexture();
    GraphResourceIdTexture getSpecularGraphTexture();

    explicit Frontend(RenderGraph& graph, Backend& backend);
    ~Frontend();

    void addObjectToScene(GeometryHandle geometry, std::optional<MaterialHandle> material,
        const Transform& transform = Transform {}, const BitMask& layer = BitMask {});
    void addObjectToScene(RenderItem item);

    inline void removeMaterialID(MaterialHandle material) { m_frameContext.removeMaterial(material); }
    void notifyMaterialUpdated(MaterialHandle material, const MaterialRenderInfo& info);
    void clearContext();

    void addFrameInvocationsToContext(RenderGraph& graph);

    LightManager& getLightManager() { return m_lightManager; }

    RenderPassId getPbrPassId(){return m_passTransparentPBRId;}

    
private:
    Backend& m_backend;
    EnvironmentMapSource m_environmentMap;
    TextureHandle m_environmentSourceTexture;
    bool m_hasEnvironmentMap = false;

    GraphResourceIdTexture m_environmentTextureId;
    GraphResourceIdTexture m_irradianceTextureId;
    GraphResourceIdTexture m_specularTextureId;
    TextureHandle  m_brdfLookupTextureHandle;

    std::shared_ptr<int> m_lifetimeToken = std::make_shared<int>(0);

    void addShadowViews(RenderGraph& graph);
    void addTextureViews(
        RenderGraph& graph,
        const RenderView& view,
        const std::vector<RenderItem>& items, 
        bool transparent);
    void addSceneView(
        RenderGraph& graph,
        const RenderView& view);
    static void configureView(
        RenderGraph& graph, 
        RenderPassInvocation& invocation, 
        const RenderView& view);

    GeometryHandle m_postProcessingQuad;
    GeometryHandle m_skyboxCube;

    LightManager m_lightManager;
    SceneContext m_frameContext;
    std::vector<Camera> m_frameCameras;

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
