#include "Frontend.hpp"

#include "Log.hpp"

namespace Galaxy {
char* copyString(const std::string& str)
{
    char* cstr = new char[str.size() + 1];
    std::strcpy(cstr, str.c_str());
    return cstr;
}

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
            auto views = device->getViews();
            auto projection = device->getProjection();
            auto position = vec3(device->transform[3]);

            // Cubemap
            if(views.size() == 6){
                for(int i=0; i < views.size(); i++){
                    bindFrameBuffer(device->targetFramebuffer, i);
                    if(!device->noClear)
                        clear(clearColor);
                    
                    setViewport(device->viewportPosition, device->viewportDimmmensions);
                    setViewMatrix(views[i]);
                    setProjectionMatrix(projection);
                    if(device->renderScene){
                        changeUsedProgram(ProgramType::PBR);
                        dumpCommandsToBuffer(position);
                    }
                }
                m_frontBuffer->insert(m_frontBuffer->end(), device->customPostCommands.begin(), device->customPostCommands.end());
                unbindFrameBuffer(device->targetFramebuffer, true);
            } else {
                bindFrameBuffer(device->targetFramebuffer, -1, device->targetDepthLayer);
                
                if(!device->noClear)
                    clear(clearColor);
                
                setViewport(device->viewportPosition, device->viewportDimmmensions);
                setViewMatrix(views[0]);
                setProjectionMatrix(projection);
                if(device->renderScene){
                    changeUsedProgram(ProgramType::PBR);
                    dumpCommandsToBuffer(position);
                }
                m_frontBuffer->insert(m_frontBuffer->end(), device->customPostCommands.begin(), device->customPostCommands.end());
                unbindFrameBuffer(device->targetFramebuffer, false);
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

void Frontend::submit(renderID meshID)
{
    RawDrawCommand drawCommand;
    drawCommand.instanceID = meshID;
    pushCommand(drawCommand);
}

void Frontend::submit(renderID meshID, const Transform& transform)
{
    DrawCommand drawCommand;
    drawCommand.instanceId = meshID;
    drawCommand.model      = transform.getGlobalModelMatrix();
    pushCommand(drawCommand);
}

void Frontend::clear(math::vec4& color)
{
    ClearCommand clearCommand;
    clearCommand.color = color;
    pushCommand(clearCommand);
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
        m_frameDevices.back()->customPostCommands.push_back(command);
    } else {
        m_frontBuffer->push_back(command);
    }
}

void Frontend::saveFrameBuffer(renderID framebufferID, std::string& path)
{
    SaveFrameBufferCommand saveFramebufferC;
    char* copy = new char[path.size() + 1];
    std::strcpy(copy, path.c_str());
    saveFramebufferC.path          = copy;
    saveFramebufferC.frameBufferID = framebufferID;
    pushCommand(saveFramebufferC);
}

void Frontend::bindTexture(renderID textureInstanceID, char* uniformName, bool important)
{
    UseTextureCommand useTextureCommand;
    useTextureCommand.instanceID  = textureInstanceID;
    useTextureCommand.uniformName = uniformName;
    useTextureCommand.important = important;
    pushCommand(useTextureCommand);
}

void Frontend::attachTextureToColorFramebuffer(renderID textureID, renderID framebufferID, int attachmentIdx)
{
    AttachTextureToFramebufferCommand attachCommand;
    attachCommand.textureID     = textureID;
    attachCommand.framebufferID = framebufferID;
    attachCommand.attachmentIdx = attachmentIdx;
    pushCommand(attachCommand);
}

void Frontend::attachTextureToDepthFramebuffer(renderID textureID, renderID framebufferID)
{
    AttachTextureToFramebufferCommand attachCommand;
    attachCommand.textureID     = textureID;
    attachCommand.framebufferID = framebufferID;
    attachCommand.attachmentIdx = -1;
    pushCommand(attachCommand);
}

void Frontend::attachCubemapToFramebuffer(renderID cubemapID, renderID framebufferID, int colorIdx)
{
    AttachCubemapToFramebufferCommand attachCommand;
    attachCommand.cubemapID     = cubemapID;
    attachCommand.framebufferID = framebufferID;
    attachCommand.colorIdx      = colorIdx;
    pushCommand(attachCommand);
}

void Frontend::useCubemap(renderID cubemapInstanceID, char* uniformName)
{
    UseCubemapCommand useCubemapCommand;
    useCubemapCommand.instanceID  = cubemapInstanceID;
    useCubemapCommand.uniformName = uniformName;
    pushCommand(useCubemapCommand);
}

void Frontend::bindFrameBuffer(renderID frameBufferInstanceID, int cubemapFaceIdx, int depthLayerIdx)
{
    BindFrameBufferCommand typeCommand;
    typeCommand.frameBufferID  = frameBufferInstanceID;
    typeCommand.cubemapFaceIdx = cubemapFaceIdx;
    typeCommand.depthLayerIdx  = depthLayerIdx;
    typeCommand.bind           = true;
    m_frontBuffer->push_back(typeCommand);
}

void Frontend::unbindFrameBuffer(renderID frameBufferInstanceID, bool cubemap)
{
    BindFrameBufferCommand typeCommand;
    typeCommand.frameBufferID  = frameBufferInstanceID;
    typeCommand.cubemapFaceIdx = cubemap ? 0 : -1;
    typeCommand.bind           = false;
    m_frontBuffer->push_back(typeCommand);
}

void Frontend::changeUsedProgram(ProgramType program)
{
    SetActiveProgramCommand setActiveProgramCommand;
    setActiveProgramCommand.program = program;

    pushCommand(setActiveProgramCommand);
}

void Frontend::setUniform(std::string uniformName, bool value)
{
    SetUniformCommand uniformCommand;
    uniformCommand.uniformName = copyString(uniformName);
    uniformCommand.type        = BOOL;
    uniformCommand.valueBool   = value;
    pushCommand(uniformCommand);
}

void Frontend::setUniform(std::string uniformName, float value)
{
    SetUniformCommand uniformCommand;
    uniformCommand.uniformName = copyString(uniformName);
    uniformCommand.type        = FLOAT;
    uniformCommand.valueFloat  = value;
    pushCommand(uniformCommand);
}

void Frontend::setUniform(std::string uniformName, int value)
{
    SetUniformCommand uniformCommand;
    uniformCommand.uniformName = copyString(uniformName);
    uniformCommand.type        = INT;
    uniformCommand.valueInt    = value;
    pushCommand(uniformCommand);
}

void Frontend::setUniform(std::string uniformName, mat4 value)
{
    SetUniformCommand uniformCommand;
    uniformCommand.uniformName = copyString(uniformName);
    uniformCommand.type        = MAT4;
    uniformCommand.matrixValue = value;
    pushCommand(uniformCommand);
}

void Frontend::setUniform(std::string uniformName, vec3 value)
{
    SetUniformCommand uniformCommand;
    uniformCommand.uniformName = copyString(uniformName);
    uniformCommand.type        = VEC3;
    uniformCommand.valueVec3.x = value.x;
    uniformCommand.valueVec3.y = value.y;
    uniformCommand.valueVec3.z = value.z;
    pushCommand(uniformCommand);
}

void Frontend::setUniform(std::string uniformName, ivec3 value)
{
    SetUniformCommand uniformCommand;
    uniformCommand.uniformName  = copyString(uniformName);
    uniformCommand.type         = IVEC3;
    uniformCommand.valueIVec3.x = value.x;
    uniformCommand.valueIVec3.y = value.y;
    uniformCommand.valueIVec3.z = value.z;

    pushCommand(uniformCommand);
}

void Frontend::setUniform(std::string uniformName, vec2 value)
{
    SetUniformCommand uniformCommand;
    uniformCommand.uniformName = copyString(uniformName);
    uniformCommand.type        = VEC2;
    uniformCommand.valueVec2.x = value.r;
    uniformCommand.valueVec2.y = value.g;
    pushCommand(uniformCommand);
}

void Frontend::bindUBO(renderID id, unsigned int idx)
{
    BindUBOCommand bindComm;
    bindComm.idx   = idx;
    bindComm.uboID = id;

    pushCommand(bindComm);
}

void Frontend::setFramebufferAsTextureUniform(renderID framebufferID, std::string uniformName, int textureIdx)
{
    SetFramebufferAsTextureUniformCommand setTextureCommand;
    setTextureCommand.framebufferID = framebufferID;
    setTextureCommand.uniformName   = copyString(uniformName);
    setTextureCommand.textureIdx    = textureIdx;
    pushCommand(setTextureCommand);
}

void Frontend::setFramebufferAsCubemapUniform(renderID framebufferID, std::string uniformName, int colorIdx)
{
    SetFramebufferAsTextureUniformCommand setTextureCommand;
    setTextureCommand.framebufferID = framebufferID;
    setTextureCommand.uniformName   = copyString(uniformName);
    setTextureCommand.textureIdx    = colorIdx;
    setTextureCommand.aboutCubemap  = true;
    pushCommand(setTextureCommand);
}

void Frontend::setViewport(vec2 position, vec2 dimmension)
{
    SetViewportCommand setViewportCommand;
    setViewportCommand.position = position;
    setViewportCommand.size     = dimmension;
    pushCommand(setViewportCommand);
}

void Frontend::resizeTexture(renderID textureID, unsigned int width, unsigned int height)
{
    UpdateTextureCommand update;
    update.targetID = textureID;
    update.width    = width;
    update.height   = height;
    pushCommand(update);
}

void Frontend::setTextureFormat(renderID textureID, TextureFormat format)
{
    UpdateTextureCommand update;
    update.targetID  = textureID;
    update.newFormat = format;
    pushCommand(update);
}

void Frontend::updateCubemap(renderID targetID, unsigned int resolution)
{
    UpdateCubemapCommand update;
    update.targetID   = targetID;
    update.resolution = resolution;
    pushCommand(update);
}

void Frontend::addDebugMsg(std::string message)
{
    DebugMsgCommand debug;
    char* copy = new char[message.size() + 1];
    std::strcpy(copy, message.c_str());
    debug.msg = copy;
    pushCommand(debug);
}

void Frontend::submitDebugLine(vec3 start, vec3 end, vec3 color)
{
    DrawDebugLineCommand drawCommand;
    drawCommand.start = start;
    drawCommand.end   = end;
    pushCommand(drawCommand);
}

void Frontend::drawDebug()
{
    // RenderCommand command;
    // command.type = RenderCommandType::executeDebugCommands;

    // pushCommand(command);

    // GLX_CORE_ERROR("Not working frontend render command drawDebug");

    // TODO : COMPLETE
}

void Frontend::submitPBR(renderID meshID, renderID materialID, const Transform& transform)
{
    DrawCommand drawCommand;
    drawCommand.instanceId = meshID;
    drawCommand.model      = transform.getGlobalModelMatrix();

    m_frameContext.pushCommand(materialID, drawCommand);
}

void Frontend::dumpCommandsToBuffer(vec3& cameraPosition)
{
    std::vector<RenderCommand> opaques = m_frameContext.retrieveOpaqueRenders();
    std::vector<RenderCommand> transparents = m_frameContext.retrieveTransparentRenders(cameraPosition);


    m_frontBuffer->insert(m_frontBuffer->end(), opaques.begin(), opaques.end());
    m_frontBuffer->insert(m_frontBuffer->end(), transparents.begin(), transparents.end());
}

void Frontend::setCommandBuffer(std::vector<RenderCommand>* newBuffer)
{
    m_frontBuffer = newBuffer;
}

void Frontend::notifyMaterialUpdated(renderID materialID, bool isTransparent)
{
    m_frameContext.onMaterialUpdated(materialID, isTransparent);
}

} // namespace Galaxy
