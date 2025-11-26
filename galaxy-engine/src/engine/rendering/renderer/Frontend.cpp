#include "Frontend.hpp"

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
                attachTextureToColorFramebuffer(canva.colorTargetID, canva.framebufferID);
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

    RenderCommand command;
    command.type    = RenderCommandType::rawDraw;
    command.rawDraw = drawCommand;

    pushCommand(command);
}

void Frontend::submit(renderID meshID, const Transform& transform)
{
    DrawCommand drawCommand;
    drawCommand.instanceId = meshID;
    drawCommand.model      = transform.getGlobalModelMatrix();

    RenderCommand command;
    command.type = RenderCommandType::draw;
    command.draw = drawCommand;

    pushCommand(command);
}

void Frontend::clear(math::vec4& color)
{
    ClearCommand clearCommand;
    clearCommand.color = color;

    RenderCommand command;
    command.type  = RenderCommandType::clear;
    command.clear = clearCommand;

    pushCommand(command);
}

void Frontend::setViewMatrix(const math::mat4& view)
{
    SetViewCommand setViewCommand;
    setViewCommand.view = view;

    RenderCommand command;
    command.type    = RenderCommandType::setView;
    command.setView = setViewCommand;

    m_frontBuffer->push_back(command);
}

void Frontend::setProjectionMatrix(const math::mat4& projection)
{
    SetProjectionCommand setProjectionCommand;
    setProjectionCommand.projection = projection;

    RenderCommand command;
    command.type          = RenderCommandType::setProjection;
    command.setProjection = setProjectionCommand;

    m_frontBuffer->push_back(command);
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

    RenderCommand command;
    command.type            = RenderCommandType::saveFrameBuffer;
    command.saveFrameBuffer = saveFramebufferC;

    pushCommand(command);
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

    RenderCommand command;
    command.type       = RenderCommandType::useTexture;
    command.useTexture = useTextureCommand;

    pushCommand(command);
}

void Frontend::attachTextureToColorFramebuffer(renderID textureID, renderID framebufferID)
{
    AttachTextureToFramebufferCommand attachCommand;
    attachCommand.textureID     = textureID;
    attachCommand.framebufferID = framebufferID;
    attachCommand.isDepth       = false;

    RenderCommand command;
    command.type                       = RenderCommandType::attachTextureToFramebuffer;
    command.attachTextureToFramebuffer = attachCommand;

    pushCommand(command);
}

void Frontend::attachTextureToDepthFramebuffer(renderID textureID, renderID framebufferID)
{
    AttachTextureToFramebufferCommand attachCommand;
    attachCommand.textureID     = textureID;
    attachCommand.framebufferID = framebufferID;
    attachCommand.isDepth       = true;

    RenderCommand command;
    command.type                       = RenderCommandType::attachTextureToFramebuffer;
    command.attachTextureToFramebuffer = attachCommand;

    pushCommand(command);
}

void Frontend::attachCubemapToFramebuffer(renderID cubemapID, renderID framebufferID)
{
    AttachCubemapToFramebufferCommand attachCommand;
    attachCommand.cubemapID     = cubemapID;
    attachCommand.framebufferID = framebufferID;
    RenderCommand command;
    command.type                       = RenderCommandType::attachCubemapToFramebuffer;
    command.attachCubemapToFramebuffer = attachCommand;

    pushCommand(command);
}

void Frontend::useCubemap(renderID cubemapInstanceID, char* uniformName)
{
    UseCubemapCommand useCubemapCommand;
    useCubemapCommand.instanceID  = cubemapInstanceID;
    useCubemapCommand.uniformName = uniformName;

    RenderCommand command;
    command.type       = RenderCommandType::useCubemap;
    command.useCubemap = useCubemapCommand;

    pushCommand(command);
}

void Frontend::bindFrameBuffer(renderID frameBufferInstanceID, int cubemapFaceIdx)
{
    BindFrameBufferCommand typeCommand;
    typeCommand.frameBufferID  = frameBufferInstanceID;
    typeCommand.cubemapFaceIdx = cubemapFaceIdx;

    RenderCommand command;
    command.type            = RenderCommandType::bindFrameBuffer;
    command.bindFrameBuffer = typeCommand;

    m_frontBuffer->push_back(command);
}

void Frontend::unbindFrameBuffer(renderID frameBufferInstanceID, bool cubemap)
{
    BindFrameBufferCommand typeCommand;
    typeCommand.frameBufferID  = frameBufferInstanceID;
    typeCommand.cubemapFaceIdx = cubemap ? 0 : -1;

    RenderCommand command;
    command.type            = RenderCommandType::unbindFrameBuffer;
    command.bindFrameBuffer = typeCommand;

    m_frontBuffer->push_back(command);
}

void Frontend::changeUsedProgram(ProgramType program)
{
    SetActiveProgramCommand setActiveProgramCommand;
    setActiveProgramCommand.program = program;
    RenderCommand progCommand;
    progCommand.type             = RenderCommandType::setActiveProgram;
    progCommand.setActiveProgram = setActiveProgramCommand;

    pushCommand(progCommand);
}

void Frontend::initPostProcessing(renderID frameBufferID)
{
    InitPostProcessCommand postProcComm;
    postProcComm.frameBufferID = frameBufferID;
    RenderCommand command;
    command.type            = RenderCommandType::initPostProcess;
    command.initPostProcess = postProcComm;

    // TODO: make it work with canvaRender
    pushCommand(command);
}

void Frontend::setUniform(std::string uniformName, bool value)
{
    SetUniformCommand uniformCommand;
    uniformCommand.uniformName = copyString(uniformName);
    uniformCommand.type        = BOOL;
    uniformCommand.valueBool   = value;
    RenderCommand command;
    command.type       = RenderCommandType::setUniform;
    command.setUniform = uniformCommand;

    pushCommand(command);
}

void Frontend::setUniform(std::string uniformName, float value)
{
    SetUniformCommand uniformCommand;
    uniformCommand.uniformName = copyString(uniformName);
    uniformCommand.type        = FLOAT;
    uniformCommand.valueFloat  = value;
    RenderCommand command;
    command.type       = RenderCommandType::setUniform;
    command.setUniform = uniformCommand;

    pushCommand(command);
}

void Frontend::setUniform(std::string uniformName, mat4 value)
{
    SetUniformCommand uniformCommand;
    uniformCommand.uniformName = copyString(uniformName);
    uniformCommand.type        = MAT4;
    uniformCommand.matrixValue = value;
    RenderCommand command;
    command.type       = RenderCommandType::setUniform;
    command.setUniform = uniformCommand;

    pushCommand(command);
}

void Frontend::setUniform(std::string uniformName, vec3 value)
{
    SetUniformCommand uniformCommand;
    uniformCommand.uniformName = copyString(uniformName);
    uniformCommand.type        = VEC3;
    uniformCommand.valueVec3.x = value.r;
    uniformCommand.valueVec3.y = value.g;
    uniformCommand.valueVec3.z = value.b;
    RenderCommand command;
    command.type       = RenderCommandType::setUniform;
    command.setUniform = uniformCommand;

    pushCommand(command);
}

void Frontend::setUniform(std::string uniformName, vec2 value)
{
    SetUniformCommand uniformCommand;
    uniformCommand.uniformName = copyString(uniformName);
    uniformCommand.type        = VEC2;
    uniformCommand.valueVec2.x = value.r;
    uniformCommand.valueVec2.y = value.g;
    RenderCommand command;
    command.type       = RenderCommandType::setUniform;
    command.setUniform = uniformCommand;

    pushCommand(command);
}

void Frontend::setUnicolorObjectColor(const vec3& color)
{
    SetUniformCommand uniformCommand;
    uniformCommand.uniformName = (char*)"objectColor";
    uniformCommand.type        = VEC3;
    uniformCommand.valueVec3.x = color.r;
    uniformCommand.valueVec3.y = color.g;
    uniformCommand.valueVec3.z = color.b;
    RenderCommand command;
    command.type       = RenderCommandType::setUniform;
    command.setUniform = uniformCommand;

    pushCommand(command);
}

void Frontend::setViewport(vec2 position, vec2 dimmension)
{
    SetViewportCommand setViewportCommand;
    setViewportCommand.position = position;
    setViewportCommand.size     = dimmension;
    RenderCommand command;
    command.type        = RenderCommandType::setViewport;
    command.setViewport = setViewportCommand;

    pushCommand(command);
}

void Frontend::updateCubemap(renderID targetID, unsigned int resolution)
{
    UpdateCubemapCommand update;
    update.targetID   = targetID;
    update.resolution = resolution;
    RenderCommand command;
    command.type          = RenderCommandType::updateCubemap;
    command.updateCubemap = update;

    pushCommand(command);
}

void Frontend::addDebugMsg(std::string message)
{
    DebugMsgCommand debug;
    char* copy = new char[message.size() + 1];
    std::strcpy(copy, message.c_str());
    debug.msg = copy;

    RenderCommand command;
    command.type     = RenderCommandType::debugMsg;
    command.debugMsg = debug;

    pushCommand(command);
}

void Frontend::submitPBR(renderID meshID, renderID materialID, const Transform& transform)
{
    DrawCommand drawCommand;
    drawCommand.instanceId = meshID;
    drawCommand.model      = transform.getGlobalModelMatrix();

    RenderCommand command;
    command.type = RenderCommandType::draw;
    command.draw = drawCommand;

    // TODO: Test if it works
    if (m_currentFramebufferFormat == FramebufferTextureFormat::DEPTH24STENCIL8) {
        if (!m_materialsTransparency[materialID]) {
            pushCommand(command);
        }
    } else {
        m_canvas[m_currentCanvaIdx].materialToSubmitCommand[materialID].push_back(command);
    }
}

vec3 Frontend::DistCompare::camPosition = vec3(0);

bool Frontend::DistCompare::operator()(const std::pair<renderID, RenderCommand>& a, const std::pair<renderID, RenderCommand>& b) const
{
    return (camPosition - vec3(a.second.draw.model[3])).length() < (camPosition - vec3(b.second.draw.model[3])).length();
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

            RenderCommand command;
            command.type         = RenderCommandType::bindMaterial;
            command.bindMaterial = bindMaterialCommand;

            m_frontBuffer->push_back(command);

            for (auto& meshCommand : queue.second) {
                m_frontBuffer->push_back(meshCommand);
            }
        }
    }

    DepthMaskCommand depthMask;
    depthMask.state = false;
    RenderCommand depthCommand;
    depthCommand.type      = RenderCommandType::depthMask;
    depthCommand.depthMask = depthMask;

    m_frontBuffer->push_back(depthCommand);

    while (!transparentPQ.empty()) {
        BindMaterialCommand bindMaterialCommand;
        bindMaterialCommand.materialRenderID = transparentPQ.top().first;

        RenderCommand command;
        command.type         = RenderCommandType::bindMaterial;
        command.bindMaterial = bindMaterialCommand;

        m_frontBuffer->push_back(command);
        m_frontBuffer->push_back(transparentPQ.top().second);

        transparentPQ.pop();
    }

    depthMask.state        = true;
    depthCommand.depthMask = depthMask;
    m_frontBuffer->push_back(depthCommand);

    for (auto& command : canva.endCommands)
        m_frontBuffer->push_back(command);
}

void Frontend::setCommandBuffer(std::vector<RenderCommand>* newBuffer)
{
    m_frontBuffer = newBuffer;
}

} // namespace Galaxy
