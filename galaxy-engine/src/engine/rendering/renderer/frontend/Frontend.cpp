#include "Frontend.hpp"

#include "Log.hpp"

namespace Galaxy {
Frontend::Frontend(std::vector<RenderCommand>* commandBuffer)
{
    m_frontBuffer = commandBuffer;
}

void Frontend::processDevices()
{
    m_addCommandsToDevice = false;

    auto clearColor = vec4(0.2, 0.2, 0.25, 1.0);
    for (auto& device : m_frameDevices) {
        if (device->useBuffer()) {
            auto projection = device->getProjection();

            // Cubemap
            if (auto* cubemap = dynamic_cast<RenderPoint*>(device.get())) {
                auto views = cubemap->getViews();

                for(int i=0; i < views.size(); i++){
                    bindCubemapFrameBuffer(device->targetCubemapFramebuffer, i);
                    if(!device->noClear)
                        clear(clearColor);
                    
                    setViewport(device->viewportPosition, cubemap->camera->dimmensions);
                    setViewMatrix(views[i]);
                    setProjectionMatrix(projection);
                    if(device->renderScene){
                        changeUsedProgram(ProgramType::PBR);
                        dumpCommandsToBuffer(cubemap->camera, device->frustumCulling);
                    }
                }
                m_frontBuffer->insert(m_frontBuffer->end(), device->customPostCommands.begin(), device->customPostCommands.end());
                unbindCubemapFrameBuffer(device->targetCubemapFramebuffer);
            } else {
                auto view = device->getView();
                bindFrameBuffer(device->targetFramebuffer, device->targetDepthLayer);
                
                if(!device->noClear)
                    clear(clearColor);
                
                setViewport(device->viewportPosition, device->viewportDimmension);
                setViewMatrix(view);
                setProjectionMatrix(projection);
                if(device->renderScene){
                    if(auto* cameraDevice = dynamic_cast<RenderCamera*>(device.get())){
                        changeUsedProgram(ProgramType::PBR);
                        dumpCommandsToBuffer(cameraDevice->camera, device->frustumCulling);
                    }
                }
                m_frontBuffer->insert(m_frontBuffer->end(), device->customPostCommands.begin(), device->customPostCommands.end());
                unbindFrameBuffer(device->targetFramebuffer);
            }
        }

        // if (canva.useBuffer) {
        //     if (canva.storeResult)
        //         saveFrameBuffer(canva.framebufferID, canva.storagePath);
        //     unbindFrameBuffer(canva.framebufferID, canva.cubemapIdx >= 0);
        // }
    }

    m_frameDevices.clear();
    m_addCommandsToDevice = true;
}

// void Frontend::storeCanvaResult(std::string& path)
// {
//     m_canvas[m_currentCanvaIdx].storeResult = true;
//     m_canvas[m_currentCanvaIdx].storagePath = path;
// }

void Frontend::submit(GeometryHandle geometry)
{
    RawDrawCommand drawCommand;
    drawCommand.geometry = geometry;
    pushCommand(std::move(drawCommand));
}

void Frontend::submit(GeometryHandle geometry, const Transform& transform)
{
    DrawCommand drawCommand;
    drawCommand.geometry   = geometry;
    drawCommand.model      = transform.getGlobalModelMatrix();
    pushCommand(std::move(drawCommand));
}

void Frontend::clear(math::vec4& color)
{
    ClearCommand clearCommand;
    clearCommand.color = color;
    pushCommand(std::move(clearCommand));
}

void Frontend::setViewMatrix(const math::mat4& view)
{
    SetViewCommand setViewCommand;
    setViewCommand.view = view;
    m_frontBuffer->push_back(setViewCommand);
}

void Frontend::setProjectionMatrix(const math::mat4& projection)
{
    SetProjectionCommand setProjectionCommand;
    setProjectionCommand.projection = projection;
    m_frontBuffer->push_back(setProjectionCommand);
}

void Frontend::pushCommand(RenderCommand command)
{
    if(m_addCommandsToDevice && m_frameDevices.size() > 0){
        m_frameDevices.back()->customPostCommands.push_back(std::move(command));
    } else {
        m_frontBuffer->push_back(std::move(command));
    }
}

void Frontend::saveFrameBuffer(FramebufferHandle framebuffer, std::string path)
{
    SaveFrameBufferCommand saveFramebufferC;
    saveFramebufferC.path        = std::move(path);
    saveFramebufferC.framebuffer = framebuffer;
    pushCommand(std::move(saveFramebufferC));
}

void Frontend::bindTexture(TextureHandle texture, std::string uniformName, bool important)
{
    UseTextureCommand useTextureCommand;
    useTextureCommand.texture     = texture;
    useTextureCommand.uniformName = std::move(uniformName);
    useTextureCommand.important = important;
    pushCommand(std::move(useTextureCommand));
}

void Frontend::attachTextureToColorFramebuffer(TextureHandle texture, FramebufferHandle framebuffer, int attachmentIdx)
{
    AttachTextureToFramebufferCommand attachCommand;
    attachCommand.texture       = texture;
    attachCommand.framebuffer   = framebuffer;
    attachCommand.attachmentIdx = attachmentIdx;
    pushCommand(std::move(attachCommand));
}

void Frontend::attachTextureToDepthFramebuffer(TextureHandle texture, FramebufferHandle framebuffer)
{
    AttachTextureToFramebufferCommand attachCommand;
    attachCommand.texture       = texture;
    attachCommand.framebuffer   = framebuffer;
    attachCommand.attachmentIdx = -1;
    pushCommand(std::move(attachCommand));
}

void Frontend::attachCubemapToFramebuffer(CubemapHandle cubemap, CubemapFramebufferHandle framebuffer, int colorIdx)
{
    AttachCubemapToFramebufferCommand attachCommand;
    attachCommand.cubemap     = cubemap;
    attachCommand.framebuffer = framebuffer;
    attachCommand.colorIdx    = colorIdx;
    pushCommand(std::move(attachCommand));
}

void Frontend::useCubemap(CubemapHandle cubemap, std::string uniformName)
{
    UseCubemapCommand useCubemapCommand;
    useCubemapCommand.cubemap     = cubemap;
    useCubemapCommand.uniformName = std::move(uniformName);
    pushCommand(std::move(useCubemapCommand));
}

void Frontend::bindMaterial(MaterialHandle material)
{
    BindMaterialCommand command;
    command.material = material;
    pushCommand(std::move(command));
}

void Frontend::bindFrameBuffer(FramebufferHandle framebuffer, int depthLayerIdx)
{
    BindFrameBufferCommand typeCommand;
    typeCommand.target        = framebuffer;
    typeCommand.depthLayerIdx = depthLayerIdx;
    typeCommand.bind          = true;
    m_frontBuffer->push_back(std::move(typeCommand));
}

void Frontend::bindCubemapFrameBuffer(CubemapFramebufferHandle framebuffer, int cubemapFaceIdx)
{
    BindFrameBufferCommand typeCommand;
    typeCommand.target         = framebuffer;
    typeCommand.cubemapFaceIdx = cubemapFaceIdx;
    typeCommand.bind           = true;
    m_frontBuffer->push_back(std::move(typeCommand));
}

void Frontend::unbindFrameBuffer(FramebufferHandle framebuffer)
{
    BindFrameBufferCommand typeCommand;
    typeCommand.target = framebuffer;
    typeCommand.bind   = false;
    m_frontBuffer->push_back(std::move(typeCommand));
}

void Frontend::unbindCubemapFrameBuffer(CubemapFramebufferHandle framebuffer)
{
    BindFrameBufferCommand typeCommand;
    typeCommand.target = framebuffer;
    typeCommand.bind   = false;
    m_frontBuffer->push_back(std::move(typeCommand));
}

void Frontend::changeUsedProgram(ProgramType program)
{
    SetActiveProgramCommand setActiveProgramCommand;
    setActiveProgramCommand.program = program;

    pushCommand(std::move(setActiveProgramCommand));
}

void Frontend::setUniform(std::string uniformName, bool value)
{
    SetUniformCommand uniformCommand;
    uniformCommand.uniformName = std::move(uniformName);
    uniformCommand.type        = BOOL;
    uniformCommand.valueBool   = value;
    pushCommand(std::move(uniformCommand));
}

void Frontend::setUniform(std::string uniformName, float value)
{
    SetUniformCommand uniformCommand;
    uniformCommand.uniformName = std::move(uniformName);
    uniformCommand.type        = FLOAT;
    uniformCommand.valueFloat  = value;
    pushCommand(std::move(uniformCommand));
}

void Frontend::setUniform(std::string uniformName, int value)
{
    SetUniformCommand uniformCommand;
    uniformCommand.uniformName = std::move(uniformName);
    uniformCommand.type        = INT;
    uniformCommand.valueInt    = value;
    pushCommand(std::move(uniformCommand));
}

void Frontend::setUniform(std::string uniformName, mat4 value)
{
    SetUniformCommand uniformCommand;
    uniformCommand.uniformName = std::move(uniformName);
    uniformCommand.type        = MAT4;
    uniformCommand.matrixValue = value;
    pushCommand(std::move(uniformCommand));
}

void Frontend::setUniform(std::string uniformName, vec3 value)
{
    SetUniformCommand uniformCommand;
    uniformCommand.uniformName = std::move(uniformName);
    uniformCommand.type        = VEC3;
    uniformCommand.valueVec3.x = value.x;
    uniformCommand.valueVec3.y = value.y;
    uniformCommand.valueVec3.z = value.z;
    pushCommand(std::move(uniformCommand));
}

void Frontend::setUniform(std::string uniformName, ivec3 value)
{
    SetUniformCommand uniformCommand;
    uniformCommand.uniformName  = std::move(uniformName);
    uniformCommand.type         = IVEC3;
    uniformCommand.valueIVec3.x = value.x;
    uniformCommand.valueIVec3.y = value.y;
    uniformCommand.valueIVec3.z = value.z;

    pushCommand(std::move(uniformCommand));
}

void Frontend::setUniform(std::string uniformName, vec2 value)
{
    SetUniformCommand uniformCommand;
    uniformCommand.uniformName = std::move(uniformName);
    uniformCommand.type        = VEC2;
    uniformCommand.valueVec2.x = value.r;
    uniformCommand.valueVec2.y = value.g;
    pushCommand(std::move(uniformCommand));
}

void Frontend::bindUBO(BufferHandle ubo, unsigned int idx)
{
    BindUBOCommand bindComm;
    bindComm.idx   = idx;
    bindComm.ubo   = ubo;

    pushCommand(std::move(bindComm));
}

void Frontend::setFramebufferAsTextureUniform(FramebufferHandle framebuffer, std::string uniformName, int textureIdx)
{
    SetFramebufferAsTextureUniformCommand setTextureCommand;
    setTextureCommand.framebuffer = framebuffer;
    setTextureCommand.uniformName = std::move(uniformName);
    setTextureCommand.textureIdx  = textureIdx;
    pushCommand(std::move(setTextureCommand));
}

void Frontend::setFramebufferAsCubemapUniform(CubemapFramebufferHandle framebuffer, std::string uniformName, int colorIdx)
{
    SetFramebufferAsTextureUniformCommand setTextureCommand;
    setTextureCommand.framebuffer = framebuffer;
    setTextureCommand.uniformName = std::move(uniformName);
    setTextureCommand.textureIdx  = colorIdx;
    pushCommand(std::move(setTextureCommand));
}

void Frontend::setViewport(vec2 position, vec2 dimmension)
{
    SetViewportCommand setViewportCommand;
    setViewportCommand.position = position;
    setViewportCommand.size     = dimmension;
    pushCommand(std::move(setViewportCommand));
}

void Frontend::resizeTexture(TextureHandle texture, unsigned int width, unsigned int height)
{
    UpdateTextureCommand update;
    update.texture = texture;
    update.width    = width;
    update.height   = height;
    pushCommand(std::move(update));
}

void Frontend::setTextureFormat(TextureHandle texture, TextureFormat format)
{
    UpdateTextureCommand update;
    update.texture   = texture;
    update.newFormat = format;
    pushCommand(std::move(update));
}

void Frontend::updateCubemap(CubemapHandle cubemap, unsigned int resolution)
{
    UpdateCubemapCommand update;
    update.cubemap    = cubemap;
    update.resolution = resolution;
    pushCommand(std::move(update));
}

void Frontend::addDebugMsg(std::string message)
{
    DebugMsgCommand debug;
    debug.msg = std::move(message);
    pushCommand(std::move(debug));
}

void Frontend::submitDebugLine(vec3 start, vec3 end, vec3 color)
{
    DrawDebugLineCommand drawCommand;
    drawCommand.start = start;
    drawCommand.end   = end;
    pushCommand(std::move(drawCommand));
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

void Frontend::dumpCommandsToBuffer(std::shared_ptr<Camera> camera, bool frustumCulling)
{
    Frustum cameraFrustum(camera.get());

    std::vector<RenderCommand> opaques, transparents;
    if(frustumCulling){
        opaques = m_frameContext.retrieveOpaqueRenders(cameraFrustum);
        transparents = m_frameContext.retrieveTransparentRenders(cameraFrustum);    
    } else{
        opaques = m_frameContext.retrieveOpaqueRenders();
        transparents = m_frameContext.retrieveTransparentRenders(camera->position);
    }
    


    m_frontBuffer->insert(m_frontBuffer->end(), opaques.begin(), opaques.end());
    m_frontBuffer->insert(m_frontBuffer->end(), transparents.begin(), transparents.end());
}

void Frontend::setCommandBuffer(std::vector<RenderCommand>* newBuffer)
{
    m_frontBuffer = newBuffer;
}

void Frontend::notifyMaterialUpdated(MaterialHandle material, bool isTransparent)
{
    m_frameContext.onMaterialUpdated(material, isTransparent);
}

} // namespace Galaxy
