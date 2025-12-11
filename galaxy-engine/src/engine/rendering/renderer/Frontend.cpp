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

void Frontend::beginCanvaNoBuffer()
{
    auto canva = RenderCanva();

    if (m_currentCanvaIdx == -1) {
        m_canvas.push_back(canva);
        m_currentCanvaIdx = 0;
    } else {
        m_canvas.insert(m_canvas.begin() + m_currentCanvaIdx, canva);
    }
}

void Frontend::beginCanva(const mat4& viewMat, const mat4& projectionMat, renderID framebufferID, FramebufferTextureFormat framebufferFormat, int cubemapIdx)
{
    auto canva = RenderCanva(viewMat, projectionMat, framebufferID, framebufferFormat, cubemapIdx);

    if (m_currentCanvaIdx == -1) {
        m_canvas.push_back(canva);
        m_currentCanvaIdx = 0;
    } else {
        m_canvas.insert(m_canvas.begin() + m_currentCanvaIdx, canva);
    }
}

void Frontend::endCanva()
{
    m_currentCanvaIdx++;
}

void Frontend::processCanvas()
{
    auto clearColor = math::vec4(0.2, 0.2, 0.25, 1.0);
    for (auto& canva : m_canvas) {
        if (canva.useBuffer) {
            bindFrameBuffer(canva.framebufferID, canva.cubemapIdx);
            if (canva.colorTargetID != 0)
                attachTextureToColorFramebuffer(canva.colorTargetID, canva.framebufferID, 0);
            if (canva.depthTargetID != 0)
                attachTextureToDepthFramebuffer(canva.depthTargetID, canva.framebufferID);
            if (canva.clearBuffer)
                clear(clearColor);
            setViewMatrix(canva.viewMat);
            setProjectionMatrix(canva.projectionMat);
            changeUsedProgram(ProgramType::PBR);
        }

        dumpCommandsToBuffer(canva);

        if (canva.useBuffer) {
            if (canva.storeResult)
                saveFrameBuffer(canva.framebufferID, canva.storagePath);
            unbindFrameBuffer(canva.framebufferID, canva.cubemapIdx >= 0);
        }
    }

    m_canvas.clear();
    m_currentCanvaIdx = 0;
}

void Frontend::linkCanvaColorToTexture(renderID textureID)
{
    // if (idx >= m_canvas[m_currentCanvaIdx].colorTargetIDs.size())
    //     m_canvas[m_currentCanvaIdx].colorTargetIDs.resize(idx + 1);
    // m_canvas[m_currentCanvaIdx].colorTargetIDs[idx] = textureID;
    m_canvas[m_currentCanvaIdx].colorTargetID = textureID;
}

void Frontend::linkCanvaDepthToTexture(renderID textureID)
{
    m_canvas[m_currentCanvaIdx].depthTargetID = textureID;
}

void Frontend::storeCanvaResult(std::string& path)
{
    m_canvas[m_currentCanvaIdx].storeResult = true;
    m_canvas[m_currentCanvaIdx].storagePath = path;
}

void Frontend::avoidCanvaBufferClear()
{
    m_canvas[m_currentCanvaIdx].clearBuffer = false;
}

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
    if (m_canvas.size() == 0 || m_currentCanvaIdx >= m_canvas.size())
        m_frontBuffer->push_back(command);
    else if (!m_canvas[m_currentCanvaIdx].materialToSubmitCommand.empty())
        m_canvas[m_currentCanvaIdx].endCommands.push_back(command);
    else
        m_canvas[m_currentCanvaIdx].commands.push_back(command);
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

mat4 Frontend::getProjectionMatrix()
{
    return m_canvas[m_currentCanvaIdx].projectionMat;
}

void Frontend::bindTexture(renderID textureInstanceID, char* uniformName)
{
    UseTextureCommand useTextureCommand;
    useTextureCommand.instanceID  = textureInstanceID;
    useTextureCommand.uniformName = uniformName;
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

void Frontend::bindFrameBuffer(renderID frameBufferInstanceID, int cubemapFaceIdx)
{
    BindFrameBufferCommand typeCommand;
    typeCommand.frameBufferID  = frameBufferInstanceID;
    typeCommand.cubemapFaceIdx = cubemapFaceIdx;
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

void Frontend::initPostProcessing(renderID frameBufferID)
{
    InitPostProcessCommand postProcComm;
    postProcComm.frameBufferID = frameBufferID;

    // TODO: make it work with canvaRender
    pushCommand(postProcComm);
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
    // TODO: Test if it works
    if (m_currentFramebufferFormat == FramebufferTextureFormat::DEPTH24STENCIL8) {
        if (!m_materialsTransparency[materialID]) {
            pushCommand(drawCommand);
        }
    } else {
        m_canvas[m_currentCanvaIdx].materialToSubmitCommand[materialID].push_back(drawCommand);
    }
}

vec3 Frontend::DistCompare::camPosition = vec3(0);

bool Frontend::DistCompare::operator()(const std::pair<renderID, RenderCommand>& a, const std::pair<renderID, RenderCommand>& b) const
{
    try {
        return (camPosition - vec3(std::get<DrawCommand>(a.second).model[3])).length() < (camPosition - vec3(std::get<DrawCommand>(b.second).model[3])).length();
    } catch (const std::bad_variant_access& ex) {
        GLX_CORE_ERROR("Wrong command type when drawing according to distance");
        return false;
    }
}

void Frontend::dumpCommandsToBuffer(RenderCanva& canva)
{
    DistCompare::camPosition = vec3(canva.viewMat[3]);

    std::priority_queue<std::pair<renderID, RenderCommand>, std::vector<std::pair<renderID, RenderCommand>>, DistCompare> transparentPQ;

    for (auto& command : canva.commands)
        m_frontBuffer->push_back(command);

    for (auto& queue : canva.materialToSubmitCommand) {
        auto matID = queue.first;
        if (m_materialsTransparency[matID]) {
            for (auto& meshCommand : queue.second) {
                auto elem = std::make_pair(matID, meshCommand);
                transparentPQ.push(elem);
            }
        } else {
            BindMaterialCommand bindMaterialCommand;
            bindMaterialCommand.materialRenderID = matID;

            m_frontBuffer->push_back(bindMaterialCommand);

            for (auto& meshCommand : queue.second) {
                m_frontBuffer->push_back(meshCommand);
            }
        }
    }

    DepthMaskCommand depthMask;
    depthMask.state = false;

    m_frontBuffer->push_back(depthMask);

    while (!transparentPQ.empty()) {
        BindMaterialCommand bindMaterialCommand;
        bindMaterialCommand.materialRenderID = transparentPQ.top().first;

        m_frontBuffer->push_back(bindMaterialCommand);
        m_frontBuffer->push_back(transparentPQ.top().second);

        transparentPQ.pop();
    }

    depthMask.state = true;
    m_frontBuffer->push_back(depthMask);

    for (auto& command : canva.endCommands)
        m_frontBuffer->push_back(command);
}

void Frontend::setCommandBuffer(std::vector<RenderCommand>* newBuffer)
{
    m_frontBuffer = newBuffer;
}

} // namespace Galaxy
