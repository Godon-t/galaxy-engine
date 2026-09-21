#include "Frontend.hpp"

#include "core/Helper.hpp"
#include "Log.hpp"
#include "rendering/renderer/Backend.hpp"

namespace Galaxy {
Frontend::Frontend(Backend& backend)
{
    m_postProcessingQuad = backend.generateQuad(vec2(2, 2), [] {});

    RenderGraphDeclaration renderGraphDeclaration;

    ///////////////////////////////// Shadow-map pass declaration
    GraphTextureDesc shadowTexture;
    shadowTexture.format = TextureFormat::DEPTH;
    shadowTexture.name = "shadow_maps";
    shadowTexture.arrayLayers = maxLightCount;

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
    opaqueColor.imported = true;

    GraphTextureDesc opaqueNormal;
    opaqueNormal.format = TextureFormat::RGBA;
    opaqueNormal.name = "gNormal";
    opaqueNormal.imported = true;
    
    GraphTextureDesc opaqueDepth;
    opaqueDepth.format = TextureFormat::RGBA;
    opaqueDepth.name = "gDepth";
    opaqueDepth.imported = true;
    
    GraphTextureDesc opaqueRoughness;
    opaqueRoughness.format = TextureFormat::RGBA;
    opaqueRoughness.name = "gRoughness";
    opaqueRoughness.imported = true;
    
    GraphTextureDesc opaqueDirect;
    opaqueDirect.format = TextureFormat::RGBA;
    opaqueDirect.name = "gDirect";
    opaqueDirect.imported = true;

    GraphTextureDesc hardwareDepth;
    hardwareDepth.name = "hardwareDepth";
    hardwareDepth.format = TextureFormat::DEPTH24STENCIL8;

    
    
    
    
    auto colorTexId = renderGraphDeclaration.addTexture(opaqueColor);
    auto normalTexId = renderGraphDeclaration.addTexture(opaqueNormal);
    auto depthTexId = renderGraphDeclaration.addTexture(opaqueDepth);
    auto roughnessTexId = renderGraphDeclaration.addTexture(opaqueRoughness);
    auto directTexId = renderGraphDeclaration.addTexture(opaqueDirect);
    auto hardwareDepthId = renderGraphDeclaration.addTexture(hardwareDepth);
    
    RenderTargetDesc step1Target;
    step1Target.colorAttachments.push_back(colorTexId);
    step1Target.colorAttachments.push_back(normalTexId);
    // GLX-TODO: write depth inside color attachment ??
    step1Target.colorAttachments.push_back(depthTexId);
    step1Target.colorAttachments.push_back(roughnessTexId);
    step1Target.colorAttachments.push_back(directTexId);
    step1Target.depthAttachment = hardwareDepthId;
    
    TargetId step1Id = renderGraphDeclaration.addTarget(step1Target);


    RenderPassDesc opaquePBRDesc;
    opaquePBRDesc.associatedProgram = backend.loadShader(engineRes("shaders/base.glsl"));
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
    
    RenderPassDesc skyboxDesc(opaquePBRDesc);
    skyboxDesc.associatedProgram = backend.loadShader(engineRes("shaders/skybox.glsl"));
    skyboxDesc.name = "skybox";
    skyboxDesc.state.clear = false;
    
    RenderPassDesc textureDesc(opaquePBRDesc);
    textureDesc.associatedProgram = backend.loadShader(engineRes("shaders/texture.glsl"));
    textureDesc.name = "texture";
    textureDesc.state.clear = false;
    
    RenderPassDesc unicolorDesc(opaquePBRDesc);
    unicolorDesc.associatedProgram = backend.loadShader(engineRes("shaders/unicolor.glsl"));
    unicolorDesc.name = "unicolor";
    unicolorDesc.state.clear = false;

    opaquePBRDesc.inputTextures.push_back({shadowTextureId, "shadowMaps"});
    transparentDesc.inputTextures.push_back({shadowTextureId, "shadowMaps"});
    
    
    GraphTextureDesc finalColor;
    finalColor.format = TextureFormat::RGBA;
    finalColor.name = "color";
    finalColor.imported = true;
    auto finalColorTexId = renderGraphDeclaration.addTexture(finalColor);
    RenderTargetDesc finalStepTarget;
    finalStepTarget.colorAttachments.push_back(finalColorTexId);
    TargetId finalStepTargetId = renderGraphDeclaration.addTarget(finalStepTarget);

    RenderPassDesc noPostProcessingDesc;
    noPostProcessingDesc.targetId = finalStepTargetId;
    noPostProcessingDesc.associatedProgram = backend.loadShader(engineRes("shaders/post_processing/none.glsl"));
    noPostProcessingDesc.name = "post_processing_no";
    noPostProcessingDesc.inputTextures.push_back({colorTexId, "sceneBuffer"});
    noPostProcessingDesc.state.clear = true;
    
    RenderPassDesc postProcessingDesc;
    postProcessingDesc.targetId = finalStepTargetId;
    postProcessingDesc.associatedProgram = backend.loadShader(engineRes("shaders/post_processing/probe_gi.glsl"));
    postProcessingDesc.name = "post_processing_a";
    postProcessingDesc.inputTextures.push_back({colorTexId, "sceneBuffer"});
    postProcessingDesc.inputTextures.push_back({normalTexId, "normalBuffer"});
    postProcessingDesc.inputTextures.push_back({depthTexId, "depthBuffer"});
    postProcessingDesc.inputTextures.push_back({roughnessTexId, "roughnessBuffer"});
    postProcessingDesc.inputTextures.push_back({directTexId, "directBuffer"});
    postProcessingDesc.state.clear = true;
    
    
    RenderPassDesc postProcessingSSGIDesc;
    postProcessingSSGIDesc.targetId = finalStepTargetId;
    postProcessingSSGIDesc.associatedProgram = backend.loadShader(engineRes("shaders/post_processing/ssgi.glsl"));
    postProcessingSSGIDesc.name = "post_processing_b";
    postProcessingSSGIDesc.inputTextures.push_back({colorTexId, "sceneBuffer"});
    postProcessingSSGIDesc.inputTextures.push_back({normalTexId, "normalBuffer"});
    postProcessingSSGIDesc.inputTextures.push_back({depthTexId, "depthBuffer"});
    postProcessingSSGIDesc.state.clear = true;
    
    m_passOpaquePBRId = renderGraphDeclaration.addPass(opaquePBRDesc);
    m_passTransparentPBRId = renderGraphDeclaration.addPass(transparentDesc);
    m_passSkyboxId = renderGraphDeclaration.addPass(skyboxDesc);
    m_passTextureId = renderGraphDeclaration.addPass(textureDesc);
    m_passUnicolorId = renderGraphDeclaration.addPass(unicolorDesc);
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

// void Frontend::storeCanvaResult(std::string& path)
// {
//     m_canvas[m_currentCanvaIdx].storeResult = true;
//     m_canvas[m_currentCanvaIdx].storagePath = path;
// }


// void Frontend::saveFrameBuffer(FramebufferHandle framebuffer, std::string path)
// {
//     SaveFrameBufferCommand saveFramebufferC;
//     saveFramebufferC.path        = std::move(path);
//     saveFramebufferC.framebuffer = framebuffer;
//     pushCommand(std::move(saveFramebufferC));
// }

// void Frontend::bindTexture(TextureHandle texture, std::string uniformName, bool important)
// {
//     UseTextureCommand useTextureCommand;
//     useTextureCommand.texture     = texture;
//     useTextureCommand.uniformName = std::move(uniformName);
//     useTextureCommand.important = important;
//     pushCommand(std::move(useTextureCommand));
// }


// void Frontend::changeUsedProgram(ProgramType program)
// {
//     SetActiveProgramCommand setActiveProgramCommand;
//     setActiveProgramCommand.program = program;

//     pushCommand(std::move(setActiveProgramCommand));
// }

// void Frontend::changeUsedProgram(ProgramHandle program)
// {
//     SetActiveProgramCommand setActiveProgramCommand;
//     setActiveProgramCommand.program = program;

//     pushCommand(std::move(setActiveProgramCommand));
// }

// void Frontend::bindUBO(BufferHandle ubo, unsigned int idx)
// {
//     BindUBOCommand bindComm;
//     bindComm.idx   = idx;
//     bindComm.ubo   = ubo;

//     pushCommand(std::move(bindComm));
// }

// void Frontend::setFramebufferAsTextureUniform(FramebufferHandle framebuffer, std::string uniformName, int textureIdx)
// {
//     SetFramebufferAsTextureUniformCommand setTextureCommand;
//     setTextureCommand.framebuffer = framebuffer;
//     setTextureCommand.uniformName = std::move(uniformName);
//     setTextureCommand.textureIdx  = textureIdx;
//     pushCommand(std::move(setTextureCommand));
// }

// void Frontend::setFramebufferAsCubemapUniform(CubemapFramebufferHandle framebuffer, std::string uniformName, int colorIdx)
// {
//     SetFramebufferAsTextureUniformCommand setTextureCommand;
//     setTextureCommand.framebuffer = framebuffer;
//     setTextureCommand.uniformName = std::move(uniformName);
//     setTextureCommand.textureIdx  = colorIdx;
//     pushCommand(std::move(setTextureCommand));
// }

// void Frontend::setViewport(vec2 position, vec2 dimmension)
// {
//     SetViewportCommand setViewportCommand;
//     setViewportCommand.position = position;
//     setViewportCommand.size     = dimmension;
//     pushCommand(std::move(setViewportCommand));
// }

// void Frontend::resizeTexture(TextureHandle texture, unsigned int width, unsigned int height)
// {
//     UpdateTextureCommand update;
//     update.texture = texture;
//     update.width    = width;
//     update.height   = height;
//     pushCommand(std::move(update));
// }

// void Frontend::setTextureFormat(TextureHandle texture, TextureFormat format)
// {
//     UpdateTextureCommand update;
//     update.texture   = texture;
//     update.newFormat = format;
//     pushCommand(std::move(update));
// }

// void Frontend::updateCubemap(CubemapHandle cubemap, unsigned int resolution)
// {
//     UpdateCubemapCommand update;
//     update.cubemap    = cubemap;
//     update.resolution = resolution;
//     pushCommand(std::move(update));
// }

// void Frontend::addDebugMsg(std::string message)
// {
//     DebugMsgCommand debug;
//     debug.msg = std::move(message);
//     pushCommand(std::move(debug));
// }

// void Frontend::submitDebugLine(vec3 start, vec3 end, vec3 color)
// {
//     DrawDebugLineCommand drawCommand;
//     drawCommand.start = start;
//     drawCommand.end   = end;
//     pushCommand(std::move(drawCommand));
// }

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
    auto lightDevices = m_lightManager.getLightsDevices();
    for (auto& device : lightDevices) {
        auto& invocation = execution.addInvocation(m_passShadowId);
        invocation.clearColor = vec4(0.0f);
        invocation.viewportPosition = device->viewportPosition;
        invocation.viewportSize = device->viewportDimmension;
        invocation.targetLayer = device->targetDepthLayer;
        invocation.viewParameters["lightSpaceMatrix"] = device->getProjection() * device->getView();
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
        opaqueInvocation.parameters.emplace("lightCount", static_cast<int>(m_lightManager.getLightCount()));

        auto& transparentInvocation = execution.addInvocation(m_passTransparentPBRId);
        configureView(transparentInvocation);
        transparentInvocation.items = std::move(transparents);
        transparentInvocation.parameters.emplace("lightCount", static_cast<int>(m_lightManager.getLightCount()));

        auto& postProcessInvocation = execution.addInvocation(m_passPostprocessId);
        configureView(postProcessInvocation);
        postProcessInvocation.items.push_back(RenderItem { m_postProcessingQuad });
    }

    m_frameDevices.clear();
    return execution;
}

FramebufferHandle Frontend::getFinalFramebuffer()
{
    return m_renderGraph.getRenderPass(m_passPostprocessId).targetFramebufferHandle;
}

} // namespace Galaxy
