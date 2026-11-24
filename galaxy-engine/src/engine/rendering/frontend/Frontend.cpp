#include "Frontend.hpp"

#include "core/Helper.hpp"
#include "Log.hpp"
#include "rendering/Renderer.hpp"
#include "resource/ResourceManager.hpp"


namespace Galaxy {
Frontend::~Frontend()
{
    m_lifetimeToken.reset();
    if (m_backend.isValid(m_environmentSourceTexture))
        m_backend.clearTexture(m_environmentSourceTexture);
}

GraphResourceIdTexture Frontend::getEnvironmentGraphTexture()
{
    return m_environmentTextureId;
}

GraphResourceIdTexture Frontend::getIrradianceGraphTexture()
{
    return m_irradianceTextureId;
}

GraphResourceIdTexture Frontend::getSpecularGraphTexture()
{
    return m_specularTextureId;
}

Frontend::Frontend(RenderGraph& graph, Backend& backend)
    : m_backend(backend)
{
    m_postProcessingQuad = backend.generateQuad(vec2(2, 2), [] {});
    m_skyboxCube = backend.generateCube(2.0f, true, [] {});

    ///////////////////////////////// Shadow-map pass declaration
    GraphResourceDescTexture shadowTexture;
    shadowTexture.format = TextureFormat::DEPTH;
    shadowTexture.name = "shadow_maps";
    shadowTexture.dimension = TextureDimension::TEXTURE_2D_ARRAY;
    shadowTexture.arrayLayers = maxLightCount;
    shadowTexture.wrapS = TextureWrap::CLAMP_TO_BORDER;
    shadowTexture.wrapT = TextureWrap::CLAMP_TO_BORDER;
    shadowTexture.textureSize = ivec2(2048);

    auto& graphDeclaration = graph.getDeclaration();

    const GraphResourceIdTexture shadowTextureId = graphDeclaration.addResource(shadowTexture);

    TargetDescRender shadowTarget;
    shadowTarget.depthAttachment = shadowTextureId;
    const RenderTargetId shadowTargetId = graphDeclaration.addTarget(shadowTarget);

    PassDescRender shadowPassDesc;
    shadowPassDesc.name = "shadow_pass";
    shadowPassDesc.associatedProgram = backend.loadShader(engineRes("shaders/shadow_depth.glsl"));
    shadowPassDesc.targetId = shadowTargetId;
    shadowPassDesc.clear = true;

    m_passShadowId = graphDeclaration.addPass(shadowPassDesc);

    ///////////////////////////////// Opaque PBR pass declaration    
    GraphResourceDescTexture opaqueColor;
    opaqueColor.format = TextureFormat::RGBA;
    opaqueColor.name = "gAlbedo";
    opaqueColor.matchViewSize = true;

    GraphResourceDescTexture opaqueNormal;
    opaqueNormal.format = TextureFormat::RGB16F;
    opaqueNormal.name = "gNormal";
    opaqueNormal.matchViewSize = true;
    opaqueNormal.filter = TextureFiltering::NEAREST;

    GraphResourceDescTexture opaqueDepth;
    opaqueDepth.format = TextureFormat::RGBA;
    opaqueDepth.name = "gDepth";
    opaqueDepth.matchViewSize = true;
    
    GraphResourceDescTexture opaqueMaterial;
    opaqueMaterial.format = TextureFormat::RGBA;
    opaqueMaterial.name = "gMaterial";
    opaqueMaterial.matchViewSize = true;

    GraphResourceDescTexture objectIds;
    objectIds.format = TextureFormat::R16UI;
    objectIds.name = "gObjectId";
    objectIds.matchViewSize = true;

    GraphResourceDescTexture hardwareDepth;
    hardwareDepth.name = "hardwareDepth";
    hardwareDepth.format = TextureFormat::DEPTH24STENCIL8;
    hardwareDepth.matchViewSize = true;

    
    
    
    
    auto albedoTexId = graphDeclaration.addResource(opaqueColor);
    auto normalTexId = graphDeclaration.addResource(opaqueNormal);
    auto materialTexId = graphDeclaration.addResource(opaqueMaterial);
    auto objectIdTexId = graphDeclaration.addResource(objectIds);
    auto hardwareDepthId = graphDeclaration.addResource(hardwareDepth);
    
    TargetDescRender step1Target;
    step1Target.colorAttachments.push_back(albedoTexId);
    step1Target.colorAttachments.push_back(normalTexId);
    step1Target.colorAttachments.push_back(materialTexId);
    step1Target.colorAttachments.push_back(objectIdTexId);
    step1Target.depthAttachment = hardwareDepthId;
    
    RenderTargetId step1Id = graphDeclaration.addTarget(step1Target);


    PassDescRender opaquePBRDesc;
    opaquePBRDesc.associatedProgram = backend.loadShader(engineRes("shaders/material.glsl"));
    opaquePBRDesc.name = "opaque_pbr";
    opaquePBRDesc.targetId = step1Id;
    opaquePBRDesc.clear = true;
    opaquePBRDesc.bindMaterials = true;

    
    
    PassDescRender transparentDesc(opaquePBRDesc);
    transparentDesc.associatedProgram = opaquePBRDesc.associatedProgram;
    transparentDesc.name = "transparent_pbr";
    // transparentDesc.state.depthTest = false;
    transparentDesc.clear = false;
    transparentDesc.bindMaterials = true;
    transparentDesc.blend = BlendMode::ALPHA;
    
    PassDescRender unicolorDesc(opaquePBRDesc);
    unicolorDesc.associatedProgram = backend.loadShader(engineRes("shaders/unicolor.glsl"));
    unicolorDesc.name = "unicolor";
    unicolorDesc.clear = false;

    // Lighting step
    GraphResourceDescTexture lightingColor;
    lightingColor.format = TextureFormat::RGBA;
    lightingColor.matchViewSize = true;
    // lightingColor.filter = TextureFiltering::NEAREST;
    lightingColor.name = "lighting";
    auto lightingColorTexId = graphDeclaration.addResource(lightingColor);
    TargetDescRender lightingStepTarget;
    lightingStepTarget.colorAttachments.push_back(lightingColorTexId);
    RenderTargetId lightingStepTargetId = graphDeclaration.addTarget(lightingStepTarget);

    GraphResourceDescTexture environmentTexture;
    environmentTexture.name = "environment";
    environmentTexture.format = TextureFormat::RGB16F;
    environmentTexture.dimension = TextureDimension::CUBEMAP;
    environmentTexture.wrapS = TextureWrap::CLAMP_TO_EDGE;
    environmentTexture.wrapT = TextureWrap::CLAMP_TO_EDGE;
    environmentTexture.wrapR = TextureWrap::CLAMP_TO_EDGE;
    environmentTexture.textureSize = ivec2(1024);
    m_environmentTextureId = graphDeclaration.addResource(environmentTexture);

    GraphResourceDescTexture irradianceTexture;
    irradianceTexture.name = "irradiance";
    irradianceTexture.format = TextureFormat::RGB16F;
    irradianceTexture.dimension = TextureDimension::CUBEMAP;
    irradianceTexture.wrapS = TextureWrap::CLAMP_TO_EDGE;
    irradianceTexture.wrapT = TextureWrap::CLAMP_TO_EDGE;
    irradianceTexture.wrapR = TextureWrap::CLAMP_TO_EDGE;
    irradianceTexture.textureSize = ivec2(32);
    m_irradianceTextureId = graphDeclaration.addResource(irradianceTexture);
    
    

    GraphResourceDescTexture specularTexture;
    specularTexture.name = "specular";
    specularTexture.format = TextureFormat::RGB16F;
    specularTexture.dimension = TextureDimension::CUBEMAP;
    specularTexture.wrapS = TextureWrap::CLAMP_TO_EDGE;
    specularTexture.wrapT = TextureWrap::CLAMP_TO_EDGE;
    specularTexture.wrapR = TextureWrap::CLAMP_TO_EDGE;
    specularTexture.textureSize = ivec2(1024);
    specularTexture.mipLevels = 5;
    specularTexture.filter = TextureFiltering::LINEAR;
    m_specularTextureId = graphDeclaration.addResource(specularTexture);

    PassDescRender lightingDesc(opaquePBRDesc);
    lightingDesc.associatedProgram = backend.loadShader(engineRes("shaders/post_processing/direct_lighting.glsl"));
    lightingDesc.name = "lighting";
    lightingDesc.targetId = lightingStepTargetId;
    lightingDesc.inputTextures.push_back({albedoTexId, "albedoBuffer"});
    lightingDesc.inputTextures.push_back({normalTexId, "normalBuffer"});
    lightingDesc.inputTextures.push_back({materialTexId, "materialBuffer"});
    lightingDesc.inputTextures.push_back({hardwareDepthId, "depthBuffer"});
    lightingDesc.inputTextures.push_back({shadowTextureId, "shadowMaps"});
    lightingDesc.inputTextures.push_back({m_environmentTextureId, "environmentMap"});
    lightingDesc.inputTextures.push_back({m_irradianceTextureId, "irradianceMap"});
    lightingDesc.inputTextures.push_back({m_specularTextureId, "specularMap"});
    lightingDesc.clear = true;
    lightingDesc.bindMaterials = false;
    lightingDesc.depthTest = false;
    lightingDesc.depthWrite = false;



    
    
    GraphResourceDescTexture finalColor;
    finalColor.format = TextureFormat::RGBA;
    finalColor.name = "color";
    finalColor.matchViewSize = true;
    auto finalColorTexId = graphDeclaration.addResource(finalColor);
    GraphResourceDescTexture finalDepth;
    finalDepth.name = "finalDepth";
    finalDepth.format = TextureFormat::DEPTH24STENCIL8;
    finalDepth.matchViewSize = true;
    auto finalDepthTexId = graphDeclaration.addResource(finalDepth);
    TargetDescRender finalStepTarget;
    finalStepTarget.colorAttachments.push_back(finalColorTexId);
    finalStepTarget.depthAttachment = finalDepthTexId;
    RenderTargetId finalStepTargetId = graphDeclaration.addTarget(finalStepTarget);
    
    
    PassDescRender noPostProcessingDesc;
    noPostProcessingDesc.targetId = finalStepTargetId;
    noPostProcessingDesc.associatedProgram = backend.loadShader(engineRes("shaders/post_processing/none.glsl"));
    noPostProcessingDesc.name = "post_processing_no";
    noPostProcessingDesc.clear = true;
    noPostProcessingDesc.inputTextures.push_back({lightingColorTexId, "sceneBuffer"});
    noPostProcessingDesc.inputTextures.push_back({m_environmentTextureId, "skybox"});
    noPostProcessingDesc.inputTextures.push_back({hardwareDepthId, "depthBuffer"});
    
    PassDescRender postProcessingDesc;
    postProcessingDesc.targetId = finalStepTargetId;
    postProcessingDesc.associatedProgram = backend.loadShader(engineRes("shaders/post_processing/probe_gi.glsl"));
    postProcessingDesc.name = "post_processing_a";
    postProcessingDesc.inputTextures.push_back({lightingColorTexId, "sceneBuffer"});
    postProcessingDesc.inputTextures.push_back({normalTexId, "normalBuffer"});
    postProcessingDesc.inputTextures.push_back({hardwareDepthId, "depthBuffer"});
    postProcessingDesc.inputTextures.push_back({materialTexId, "materialBuffer"});
    postProcessingDesc.clear = true;
    
    
    PassDescRender postProcessingSSGIDesc;
    postProcessingSSGIDesc.targetId = finalStepTargetId;
    postProcessingSSGIDesc.associatedProgram = backend.loadShader(engineRes("shaders/post_processing/ssgi.glsl"));
    postProcessingSSGIDesc.name = "post_processing_b";
    postProcessingSSGIDesc.inputTextures.push_back({lightingColorTexId, "sceneBuffer"});
    postProcessingSSGIDesc.inputTextures.push_back({normalTexId, "normalBuffer"});
    postProcessingSSGIDesc.inputTextures.push_back({hardwareDepthId, "depthBuffer"});
    postProcessingSSGIDesc.clear = true;
    
    m_passOpaquePBRId = graphDeclaration.addPass(opaquePBRDesc);
    m_passTransparentPBRId = graphDeclaration.addPass(transparentDesc);
    m_passUnicolorId = graphDeclaration.addPass(unicolorDesc);
    m_passLightingId = graphDeclaration.addPass(lightingDesc);
    // graphDeclaration.addPass(postProcessingDesc);
    // m_passPostprocessId = graphDeclaration.addPass(postProcessingSSGIDesc);
    m_passPostprocessId = graphDeclaration.addPass(noPostProcessingDesc);
    
    PassDescRender textureDesc;
    textureDesc.name = "texture";
    textureDesc.associatedProgram = backend.loadShader(engineRes("shaders/texture.glsl"));
    textureDesc.targetId = finalStepTargetId;
    textureDesc.clear = false;
    textureDesc.bindMaterials = true;
    m_passTextureId = graphDeclaration.addPass(textureDesc);
    
    PassDescRender transparentTextureDesc(textureDesc);
    transparentTextureDesc.name = "transparent_texture";
    transparentTextureDesc.blend = BlendMode::ALPHA;
    transparentTextureDesc.depthWrite = false;
    m_passTransparentTextureId = graphDeclaration.addPass(transparentTextureDesc);
    
    PassDescRender cubemapRenderDesc(textureDesc);
    cubemapRenderDesc.name = "cubemap_render";
    cubemapRenderDesc.associatedProgram = backend.loadShader(engineRes("shaders/debug/cubemap_render.glsl"));
    m_passCubemapRenderId = graphDeclaration.addPass(cubemapRenderDesc);
    
    PassDescRender transparentCubemapDesc(cubemapRenderDesc);
    transparentCubemapDesc.name = "transparent_cubemap";
    transparentCubemapDesc.blend = BlendMode::ALPHA;
    transparentCubemapDesc.depthWrite = false;
    m_passTransparentCubemapId = graphDeclaration.addPass(transparentCubemapDesc);
    
    
    // PassDescRender computeOcahedralDesc;
    // computeOcahedralDesc.associatedProgram = backend.loadShader(engineRes("shaders/compute_octahedral.glsl"));

    auto brdfImage = ResourceManager::getInstance().load<Image>(engineRes("imgs/brdf.gres"));
    m_brdfLookupTextureHandle = backend.instantiateTexture(brdfImage);
    backend.setTextureWrap(m_brdfLookupTextureHandle, TextureWrap::CLAMP_TO_EDGE, TextureWrap::CLAMP_TO_EDGE);
}

void Frontend::addObjectToScene(GeometryHandle geometry, std::optional<MaterialHandle> material,
    const Transform& transform, const BitMask& layer)
{
    RenderItem item;
    item.geometry = geometry;
    item.material = material;
    item.visibilityLayer = layer;
    item.bounds = m_backend.getMeshBoundingVolume(geometry);
    item.transform = transform;
    m_frameContext.push(std::move(item));
}
    
void Frontend::addObjectToScene(RenderItem item)
{
    item.bounds = m_backend.getMeshBoundingVolume(item.geometry);
    m_frameContext.push(std::move(item));
}

void Frontend::notifyMaterialUpdated(MaterialHandle material, const MaterialRenderInfo& info)
{
    m_frameContext.onMaterialUpdated(material, info);
}

void Frontend::clearContext()
{
    m_frameContext.clear();
}

void Frontend::addFrameInvocationsToContext(RenderGraph& graph)
{
    addShadowViews(graph);

    for (const auto& camera : m_frameCameras) {
        for (const auto& view : camera.getViews())
            addSceneView(graph, view);
    }

    m_frameCameras.clear();
}

void Frontend::addShadowViews(RenderGraph& graph)
{
    // A single compiled shadow pass is invoked once for every light view.
    auto lightsData = m_lightManager.getLightsData();
    vec2 viewportDimmension = vec2(2048, 2048);
    for (auto& light : lightsData) {
        if(!light.castShadow)
            continue;
        auto& invocation = graph.generateInvocation(m_passShadowId);
        invocation.clearColor = vec4(0.0f);
        // invocation.viewportPosition = light.;
        invocation.viewportSize = viewportDimmension;
        invocation.targetLayer = light.shadowMapLayer;
        auto proj = light.getProjection(viewportDimmension);
        invocation.parameters["lightSpaceMatrix"] = proj * CameraManager::getInstance().processViewMatrix(light.transformationMatrix);
        invocation.items = m_frameContext.retrieveOpaqueRenders(BitMask{1});
    }
}

void Frontend::configureView(RenderGraph& graph, RenderPassInvocation& invocation, const RenderView& view)
{
    invocation.clearColor = vec4(0.2, 0.2, 0.25, 1.0);
    invocation.viewportPosition = view.viewportPosition;
    invocation.viewportSize = view.viewportSize;
    invocation.parameters["projection"] = view.projectionMatrix;
    invocation.parameters["view"] = view.viewMatrix;
}

void Frontend::addSceneView(RenderGraph& graph, const RenderView& view)
{
    std::vector<RenderItem> opaques;
    std::vector<RenderItem> transparents;
    if (view.frustumCulling) {
        const Frustum frustum(view);
        opaques = m_frameContext.retrieveOpaqueRenders(frustum, view.visibilityMask);
        transparents = m_frameContext.retrieveTransparentRenders(frustum, view.visibilityMask);
    } else {
        opaques = m_frameContext.retrieveOpaqueRenders(view.visibilityMask);
        transparents = m_frameContext.retrieveTransparentRenders(view.position, view.visibilityMask);
    }

    auto& opaqueInvocation = graph.generateInvocation(m_passOpaquePBRId);
    configureView(graph, opaqueInvocation, view);
    for (const auto& item : opaques) {
        if (m_frameContext.getMaterialInfo(item.material).type == MaterialType::PBR)
            opaqueInvocation.items.push_back(item);
    }

    auto& transparentInvocation = graph.generateInvocation(m_passTransparentPBRId);
    configureView(graph, transparentInvocation, view);
    for (const auto& item : transparents) {
        if (m_frameContext.getMaterialInfo(item.material).type == MaterialType::PBR)
            transparentInvocation.items.push_back(item);
    }

    auto& lightInvocation = graph.generateInvocation(m_passLightingId);
    if(m_lightManager.isDirty()){
        auto updateCommand = m_lightManager.getLightUboUpdate();
        lightInvocation.updates.push_back(updateCommand);
    }
    lightInvocation.items.push_back(RenderItem { m_postProcessingQuad });
    lightInvocation.uniformBindings.push_back({m_lightManager.getLightUboHandle(), 0});
    lightInvocation.parameters.emplace("lightCount", static_cast<int>(m_lightManager.getLightCount()));
    lightInvocation.parameters.emplace("inverseView", inverse(view.viewMatrix));
    lightInvocation.parameters.emplace("inverseProjection", inverse(view.projectionMatrix));
    lightInvocation.parameters.emplace("cameraPos", view.position);
    lightInvocation.parameters.emplace("hasEnvironmentMap", m_hasEnvironmentMap);
    auto textureBinding = TextureBinding();
    textureBinding.samplerName = "brdfLookupMap";
    textureBinding.texture = m_brdfLookupTextureHandle;
    lightInvocation.textureBindings.push_back(textureBinding);
    lightInvocation.viewportPosition = view.viewportPosition;
    lightInvocation.viewportSize = view.viewportSize;


    auto& postProcessInvocation = graph.generateInvocation(m_passPostprocessId);
    configureView(graph, postProcessInvocation, view);
    postProcessInvocation.targetFramebuffer = view.targetFramebuffer;
    postProcessInvocation.targetLayer = view.targetLayer;
    postProcessInvocation.items.push_back(RenderItem { m_postProcessingQuad });

    postProcessInvocation.parameters.emplace("inverseView", inverse(view.viewMatrix));
    postProcessInvocation.parameters.emplace("inverseProjection", inverse(view.projectionMatrix));
    postProcessInvocation.parameters.emplace("cameraPos", view.position);

    addTextureViews(graph, view, opaques, false);
    addTextureViews(graph, view, transparents, true);
}

void Frontend::addTextureViews(
    RenderGraph& graph,
    const RenderView& view,
    const std::vector<RenderItem>& items, 
    bool transparent)
{
    // Preserve the sorted order across 2D textures and cubemaps when blending.
    for (const auto& item : items) {
        const auto& info = m_frameContext.getMaterialInfo(item.material);
        if (info.type != MaterialType::Texture)
            continue;

        const auto pass = info.isCubemap
            ? (transparent ? m_passTransparentCubemapId : m_passCubemapRenderId)
            : (transparent ? m_passTransparentTextureId : m_passTextureId);
        auto& invocation = graph.generateInvocation(pass);
        configureView(graph, invocation, view);
        invocation.targetFramebuffer = view.targetFramebuffer;
        invocation.targetLayer = view.targetLayer;
        invocation.items.push_back(item);
    }
}

} // namespace Galaxy
