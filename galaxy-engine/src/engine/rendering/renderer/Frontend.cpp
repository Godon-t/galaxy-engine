#include "Frontend.hpp"

namespace Galaxy {
Frontend::Frontend(std::vector<RenderCommand>* commandBuffer)
{
    m_frontBuffer = commandBuffer;
}

void Frontend::submit(renderID meshID, const Transform& transform)
{
    DrawCommand drawCommand;
    drawCommand.instanceId = meshID;
    drawCommand.model      = transform.getGlobalModelMatrix();

    RenderCommand command;
    command.type = RenderCommandType::draw;
    command.draw = drawCommand;

    m_frontBuffer->push_back(command);
}

void Frontend::clear(math::vec4& color)
{
    ClearCommand clearCommand;
    clearCommand.color = color;

    RenderCommand command;
    command.type  = RenderCommandType::clear;
    command.clear = clearCommand;

    m_frontBuffer->push_back(command);
}

void Frontend::setViewMatrix(math::mat4& view)
{
    SetViewCommand setViewCommand;
    setViewCommand.view = view;

    RenderCommand command;
    command.type    = RenderCommandType::setView;
    command.setView = setViewCommand;

    m_frontBuffer->push_back(command);

    m_viewMat = view;

    DistCompare::camPosition = vec3(view[3]);
}

void Frontend::setProjectionMatrix(math::mat4& projection)
{
    SetProjectionCommand setProjectionCommand;
    setProjectionCommand.projection = projection;

    RenderCommand command;
    command.type          = RenderCommandType::setProjection;
    command.setProjection = setProjectionCommand;

    m_frontBuffer->push_back(command);

    m_projMat = projection;
}

void Frontend::bindTexture(renderID textureInstanceID, char* uniformName)
{
    BindTextureCommand bindTextureCommand;
    bindTextureCommand.instanceID  = textureInstanceID;
    bindTextureCommand.uniformName = uniformName;

    RenderCommand command;
    command.type        = RenderCommandType::bindTexture;
    command.bindTexture = bindTextureCommand;

    m_frontBuffer->push_back(command);
}

void Frontend::bindCubemap(renderID cubemapInstanceID, char* uniformName)
{
    BindCubemapCommand bindCubemapCommand;
    bindCubemapCommand.instanceID  = cubemapInstanceID;
    bindCubemapCommand.uniformName = uniformName;

    RenderCommand command;
    command.type        = RenderCommandType::bindCubemap;
    command.bindCubemap = bindCubemapCommand;

    m_frontBuffer->push_back(command);
}

void Frontend::changeUsedProgram(ProgramType program)
{
    SetActiveProgramCommand setActiveProgramCommand;
    setActiveProgramCommand.program = program;
    RenderCommand progCommand;
    progCommand.type             = RenderCommandType::setActiveProgram;
    progCommand.setActiveProgram = setActiveProgramCommand;

    m_frontBuffer->push_back(progCommand);
    setProjectionMatrix(m_projMat);
    setViewMatrix(m_viewMat);
}

void Frontend::submitPBR(renderID meshID, renderID materialID, const Transform& transform)
{
    DrawCommand drawCommand;
    drawCommand.instanceId = meshID;
    drawCommand.model      = transform.getGlobalModelMatrix();

    RenderCommand command;
    command.type = RenderCommandType::draw;
    command.draw = drawCommand;

    m_materialToSubmitCommand[materialID].push_back(command);
}

vec3 Frontend::DistCompare::camPosition = vec3(0);

bool Frontend::DistCompare::operator()(const std::pair<renderID, RenderCommand>& a, const std::pair<renderID, RenderCommand>& b) const
{
    return (camPosition - vec3(a.second.draw.model[3])).length() < (camPosition - vec3(b.second.draw.model[3])).length();
}

void Frontend::dumpCommandsToBuffer()
{
    changeUsedProgram(ProgramType::PBR);

    std::priority_queue<std::pair<renderID, RenderCommand>, std::vector<std::pair<renderID, RenderCommand>>, DistCompare> transparentPQ;

    for (auto& queue : m_materialToSubmitCommand) {
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

    m_materialToSubmitCommand.clear();
}

void Frontend::setCommandBuffer(std::vector<RenderCommand>* newBuffer)
{
    m_frontBuffer = newBuffer;
}

} // namespace Galaxy
