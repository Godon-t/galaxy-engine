#include "Frontend.hpp"

#include "core/Helper.hpp"
#include "Log.hpp"
#include "rendering/renderer/Renderer.hpp"

#include <variant>

namespace Galaxy {
void Frontend::setEnvironment(ResourceHandle<Environment> environment)
{
    environment.getResource().onLoaded([this, environment] {
        auto& skyboxPass = m_renderGraph.getRenderPass(m_passPostprocessId);
        const GraphTextureHandle skyboxTexture = skyboxPass.findInputTexture("skybox");

        std::visit(
            [environment](const auto& typedHandle) {
                using Handle = std::decay_t<decltype(typedHandle)>;
                if constexpr (std::is_same_v<Handle, CubemapHandle>) {
                    Renderer::getInstance().getBackend().setCubemapData(typedHandle, environment.getResource().getSkybox());
                } else {
                    GLX_CORE_WARN("Wrong texturehandle for skybox cubemap data");
                }
            },
            skyboxTexture);
    });
}

Frontend::Frontend(Backend& backend)
{
    m_postProcessingQuad = backend.generateQuad(vec2(2, 2), [] {});
    m_skyboxCube = backend.generateCube(2.0f, true, [] {});

    RenderGraphDeclaration renderGraphDeclaration;

    ///////////////////////////////// Shadow-map pass declaration
    GraphTextureDesc shadowTexture;
    shadowTexture.format = TextureFormat::DEPTH;
    shadowTexture.name = "shadow_maps";
    shadowTexture.dimension = TextureDimension::Texture2DArray;
    shadowTexture.arrayLayers = maxLightCount;
    shadowTexture.wrapS = TextureWrap::CLAMP_TO_BORDER;
    shadowTexture.wrapT = TextureWrap::CLAMP_TO_BORDER;

    const GraphTextureId shadowTextureId = renderGraphDeclaration.addTexture(shadowTexture);

    RenderTargetDesc shadowTarget;
    shadowTarget.depthAttachment = shadowTextureId;
    const TargetId shadowTargetId = renderGraphDeclaration.addTarget(shadowTarget);

    RenderPassDesc shadowPassDesc;
    shadowPassDesc.name = "shadow_pass";
    shadowPassDesc.associatedProgram = backend.loadShader(engineRes("shaders/shadow_depth.glsl"));
    shadowPassDesc.targetId = shadowTargetId;
    shadowPassDesc.state.clear = true;

    m_passShadowId = renderGraphDeclaration.addPass(shadowPassDesc);

    ///////////////////////////////// Opaque PBR pass declaration    
    GraphTextureDesc opaqueColor;
    opaqueColor.format = TextureFormat::RGBA;
    opaqueColor.name = "gAlbedo";

    GraphTextureDesc opaqueNormal;
    opaqueNormal.format = TextureFormat::RGBA;
    opaqueNormal.name = "gNormal";

    GraphTextureDesc opaqueDepth;
    opaqueDepth.format = TextureFormat::RGBA;
    opaqueDepth.name = "gDepth";
    
    GraphTextureDesc opaqueMaterial;
    opaqueMaterial.format = TextureFormat::RGBA;
    opaqueMaterial.name = "gMaterial";

    GraphTextureDesc hardwareDepth;
    hardwareDepth.name = "hardwareDepth";
    hardwareDepth.format = TextureFormat::DEPTH24STENCIL8;

    
    
    
    
    auto albedoTexId = renderGraphDeclaration.addTexture(opaqueColor);
    auto normalTexId = renderGraphDeclaration.addTexture(opaqueNormal);
    auto materialTexId = renderGraphDeclaration.addTexture(opaqueMaterial);
    auto hardwareDepthId = renderGraphDeclaration.addTexture(hardwareDepth);
    
    RenderTargetDesc step1Target;
    step1Target.colorAttachments.push_back(albedoTexId);
    step1Target.colorAttachments.push_back(normalTexId);
    step1Target.colorAttachments.push_back(materialTexId);
    step1Target.depthAttachment = hardwareDepthId;
    
    TargetId step1Id = renderGraphDeclaration.addTarget(step1Target);


    RenderPassDesc opaquePBRDesc;
    opaquePBRDesc.associatedProgram = backend.loadShader(engineRes("shaders/material.glsl"));
    opaquePBRDesc.name = "opaque_pbr";
    opaquePBRDesc.targetId = step1Id;
    opaquePBRDesc.state.clear = true;
    opaquePBRDesc.state.supportPBR = true;

    
    
    // All 4 passes share the same framebuffer as opaquePBR 
    RenderPassDesc transparentDesc(opaquePBRDesc);
    transparentDesc.associatedProgram = opaquePBRDesc.associatedProgram;
    transparentDesc.name = "transparent_pbr";
    // transparentDesc.state.depthTest = false;
    transparentDesc.state.clear = false;
    transparentDesc.state.supportPBR = true;
    transparentDesc.state.blend = BlendMode::Alpha;
    
    RenderPassDesc textureDesc(opaquePBRDesc);
    textureDesc.associatedProgram = backend.loadShader(engineRes("shaders/texture.glsl"));
    textureDesc.name = "texture";
    textureDesc.state.clear = false;
    
    RenderPassDesc unicolorDesc(opaquePBRDesc);
    unicolorDesc.associatedProgram = backend.loadShader(engineRes("shaders/unicolor.glsl"));
    unicolorDesc.name = "unicolor";
    unicolorDesc.state.clear = false;

    // Lighting step
    GraphTextureDesc lightingColor;
    lightingColor.format = TextureFormat::RGBA;
    // lightingColor.filter = TextureFiltering::NEAREST;
    lightingColor.name = "lighting";
    auto lightingColorTexId = renderGraphDeclaration.addTexture(lightingColor);
    RenderTargetDesc lightingStepTarget;
    lightingStepTarget.colorAttachments.push_back(lightingColorTexId);
    TargetId lightingStepTargetId = renderGraphDeclaration.addTarget(lightingStepTarget);

    RenderPassDesc lightingDesc(opaquePBRDesc);
    lightingDesc.associatedProgram = backend.loadShader(engineRes("shaders/post_processing/direct_lighting.glsl"));
    lightingDesc.name = "lighting";
    lightingDesc.targetId = lightingStepTargetId;
    lightingDesc.inputTextures.push_back({albedoTexId, "albedoBuffer"});
    lightingDesc.inputTextures.push_back({normalTexId, "normalBuffer"});
    lightingDesc.inputTextures.push_back({materialTexId, "materialBuffer"});
    lightingDesc.inputTextures.push_back({hardwareDepthId, "depthBuffer"});
    lightingDesc.inputTextures.push_back({shadowTextureId, "shadowMaps"});
    lightingDesc.state.clear = true;



    
    
    GraphTextureDesc finalColor;
    finalColor.format = TextureFormat::RGBA;
    finalColor.name = "color";
    auto finalColorTexId = renderGraphDeclaration.addTexture(finalColor);
    RenderTargetDesc finalStepTarget;
    finalStepTarget.colorAttachments.push_back(finalColorTexId);
    TargetId finalStepTargetId = renderGraphDeclaration.addTarget(finalStepTarget);


    GraphTextureDesc environmentTexture;
    environmentTexture.name = "environment";
    environmentTexture.dimension = TextureDimension::Cubemap;
    environmentTexture.wrapS = TextureWrap::CLAMP_TO_EDGE;
    environmentTexture.wrapT = TextureWrap::CLAMP_TO_EDGE;
    environmentTexture.wrapR = TextureWrap::CLAMP_TO_EDGE;
    auto envTexId = renderGraphDeclaration.addTexture(environmentTexture);


    RenderPassDesc noPostProcessingDesc;
    noPostProcessingDesc.targetId = finalStepTargetId;
    noPostProcessingDesc.associatedProgram = backend.loadShader(engineRes("shaders/post_processing/none.glsl"));
    noPostProcessingDesc.name = "post_processing_no";
    noPostProcessingDesc.state.clear = true;
    noPostProcessingDesc.inputTextures.push_back({lightingColorTexId, "sceneBuffer"});
    noPostProcessingDesc.inputTextures.push_back({envTexId, "skybox"});
    noPostProcessingDesc.inputTextures.push_back({hardwareDepthId, "depthBuffer"});
    
    RenderPassDesc postProcessingDesc;
    postProcessingDesc.targetId = finalStepTargetId;
    postProcessingDesc.associatedProgram = backend.loadShader(engineRes("shaders/post_processing/probe_gi.glsl"));
    postProcessingDesc.name = "post_processing_a";
    postProcessingDesc.inputTextures.push_back({lightingColorTexId, "sceneBuffer"});
    postProcessingDesc.inputTextures.push_back({normalTexId, "normalBuffer"});
    postProcessingDesc.inputTextures.push_back({hardwareDepthId, "depthBuffer"});
    postProcessingDesc.inputTextures.push_back({materialTexId, "materialBuffer"});
    postProcessingDesc.state.clear = true;
    
    
    RenderPassDesc postProcessingSSGIDesc;
    postProcessingSSGIDesc.targetId = finalStepTargetId;
    postProcessingSSGIDesc.associatedProgram = backend.loadShader(engineRes("shaders/post_processing/ssgi.glsl"));
    postProcessingSSGIDesc.name = "post_processing_b";
    postProcessingSSGIDesc.inputTextures.push_back({lightingColorTexId, "sceneBuffer"});
    postProcessingSSGIDesc.inputTextures.push_back({normalTexId, "normalBuffer"});
    postProcessingSSGIDesc.inputTextures.push_back({hardwareDepthId, "depthBuffer"});
    postProcessingSSGIDesc.state.clear = true;
    
    m_passOpaquePBRId = renderGraphDeclaration.addPass(opaquePBRDesc);
    m_passTransparentPBRId = renderGraphDeclaration.addPass(transparentDesc);
    m_passTextureId = renderGraphDeclaration.addPass(textureDesc);
    m_passUnicolorId = renderGraphDeclaration.addPass(unicolorDesc);
    m_passLightingId = renderGraphDeclaration.addPass(lightingDesc);
    // renderGraphDeclaration.addPass(postProcessingDesc);
    // m_passPostprocessId = renderGraphDeclaration.addPass(postProcessingSSGIDesc);
    m_passPostprocessId = renderGraphDeclaration.addPass(noPostProcessingDesc);
    
    
    CompiledRenderGraph compiledRenderGraph(renderGraphDeclaration);
    m_renderGraph.build(compiledRenderGraph, backend);

    
    // GLX-TODO: only applied on probe cubemap
    RenderPassDesc irradianceFilterDesc;
    irradianceFilterDesc.associatedProgram = backend.loadShader(engineRes("shaders/filters/irradiance.glsl"));
    
    RenderPassDesc computeOcahedralDesc;
    computeOcahedralDesc.associatedProgram = backend.loadShader(engineRes("shaders/compute_octahedral.glsl"));
}

void Frontend::drawDebug()
{
    // RenderCommand command;
    // command.type = RenderCommandType::executeDebugCommands;

    // pushCommand(command);

    // GLX_CORE_ERROR("Not working frontend render command drawDebug");

    // TODO : COMPLETE
}


void Frontend::addObjectToScene(GeometryHandle geometry, const Sphere& boundingVolume, std::optional<MaterialHandle> material, const Transform& transform)
{
    RenderItem item;
    item.geometry = geometry;
    item.material = material;
    item.bounds = boundingVolume;
    item.transform = transform;
    m_frameContext.push(std::move(item));
}


void Frontend::notifyMaterialUpdated(MaterialHandle material, bool isTransparent)
{
    m_frameContext.onMaterialUpdated(material, isTransparent);
}


RenderGraphExecution Frontend::buildFrameExecution()
{
    RenderGraphExecution execution(m_renderGraph);

    // A single compiled shadow pass is invoked once for every light view.
    auto lightsData = m_lightManager.getLightsData();
    vec2 viewportDimmension = vec2(1024, 1024);
    for (auto& light : lightsData) {
        if(!light.castShadow)
            continue;
        auto& invocation = execution.addInvocation(m_passShadowId);
        invocation.clearColor = vec4(0.0f);
        // invocation.viewportPosition = light.;
        invocation.viewportSize = viewportDimmension;
        invocation.targetLayer = light.shadowMapLayer;
        auto proj = light.getProjection(viewportDimmension);
        invocation.viewParameters["lightSpaceMatrix"] = proj * CameraManager::getInstance().processViewMatrix(light.transformationMatrix);
        invocation.items = m_frameContext.retrieveOpaqueRenders();
    }

    for (auto& device : m_frameDevices) {
        if (!device->renderScene)
            continue;

        auto* cameraDevice = dynamic_cast<RenderCamera*>(device.get());
        if (cameraDevice == nullptr) {
            GLX_CORE_ERROR("CameraDevice type not supported !");
            continue;
        }

        std::vector<RenderItem> opaques;
        std::vector<RenderItem> transparents;
        if (device->frustumCulling) {
            Frustum cameraFrustum(cameraDevice->camera.get());
            opaques = m_frameContext.retrieveOpaqueRenders(cameraFrustum);
            transparents = m_frameContext.retrieveTransparentRenders(cameraFrustum);
        } else {
            opaques = m_frameContext.retrieveOpaqueRenders();
            transparents = m_frameContext.retrieveTransparentRenders(cameraDevice->camera->position);
        }

        const mat4 projection = device->getProjection();
        const mat4 view = device->getView();
        auto configureView = [&](RenderPassInvocation& invocation) {
            invocation.clearColor = vec4(0.2, 0.2, 0.25, 1.0);
            invocation.viewportPosition = device->viewportPosition;
            invocation.viewportSize = device->viewportDimmension;
            invocation.viewParameters["projection"] = projection;
            invocation.viewParameters["view"] = view;
        };

        auto& opaqueInvocation = execution.addInvocation(m_passOpaquePBRId);
        configureView(opaqueInvocation);
        opaqueInvocation.items = std::move(opaques);
        
        auto& transparentInvocation = execution.addInvocation(m_passTransparentPBRId);
        configureView(transparentInvocation);
        transparentInvocation.items = std::move(transparents);

        auto& lightInvocation = execution.addInvocation(m_passLightingId);
        if(m_lightManager.isDirty()){
            auto updateCommand = m_lightManager.getLightUboUpdate();
            lightInvocation.updates.push_back(updateCommand);
        }
        lightInvocation.items.push_back(RenderItem { m_postProcessingQuad });
        lightInvocation.uniformBindings.push_back({m_lightManager.getLightUboHandle(), 0});
        lightInvocation.parameters.emplace("lightCount", static_cast<int>(m_lightManager.getLightCount()));
        lightInvocation.parameters.emplace("inverseView", inverse(view));
        lightInvocation.parameters.emplace("inverseProjection", inverse(projection));
        lightInvocation.parameters.emplace("cameraPos", cameraDevice->camera->position);


        auto& postProcessInvocation = execution.addInvocation(m_passPostprocessId);
        configureView(postProcessInvocation);
        postProcessInvocation.items.push_back(RenderItem { m_postProcessingQuad });

        postProcessInvocation.parameters.emplace("view", view);
        postProcessInvocation.parameters.emplace("projection", projection);
        postProcessInvocation.parameters.emplace("inverseView", inverse(view));
        postProcessInvocation.parameters.emplace("inverseProjection", inverse(projection));
        postProcessInvocation.parameters.emplace("cameraPos", cameraDevice->camera->position);
    }

    m_frameDevices.clear();
    return execution;
}

FramebufferHandle Frontend::getFinalFramebuffer()
{
    const GraphFramebufferHandle& framebuffer =
        m_renderGraph.getRenderPass(m_passPostprocessId).targetFramebufferHandle;
    const auto* texture2DFramebuffer = std::get_if<FramebufferHandle>(&framebuffer);
    if (texture2DFramebuffer == nullptr) {
        GLX_CORE_ERROR("The final render target is a cubemap framebuffer");
        return {};
    }
    return *texture2DFramebuffer;
}

} // namespace Galaxy
