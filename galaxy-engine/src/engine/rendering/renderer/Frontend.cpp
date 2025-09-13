#include "Frontend.hpp"

namespace Galaxy {
Frontend::Frontend(std::vector<RenderCommand>& commandBuffer)
    : m_frontBuffer(commandBuffer)
{
}

void Frontend::submit(renderID meshID, const Transform& transform)
{
    DrawCommand drawCommand;
    drawCommand.instanceId = meshID;
    drawCommand.model      = transform.getGlobalModelMatrix();

    RenderCommand command;
    command.type = RenderCommandType::draw;
    command.draw = drawCommand;

    m_frontBuffer.push_back(command);
}

void Frontend::clear(math::vec4& color)
{
    ClearCommand clearCommand;
    clearCommand.color = color;

    RenderCommand command;
    command.type  = RenderCommandType::clear;
    command.clear = clearCommand;

    m_frontBuffer.push_back(command);
}

void Frontend::setViewMatrix(math::mat4& view)
{
    SetViewCommand setViewCommand;
    setViewCommand.view = view;

    RenderCommand command;
    command.type    = RenderCommandType::setView;
    command.setView = setViewCommand;

    m_frontBuffer.push_back(command);

    m_viewMat = view;
}

void Frontend::setProjectionMatrix(math::mat4& projection)
{
    SetProjectionCommand setProjectionCommand;
    setProjectionCommand.projection = projection;

    RenderCommand command;
    command.type          = RenderCommandType::setProjection;
    command.setProjection = setProjectionCommand;

    m_frontBuffer.push_back(command);

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

    m_frontBuffer.push_back(command);
}

void Frontend::bindCubemap(renderID cubemapInstanceID, char* uniformName)
{
    BindCubemapCommand bindCubemapCommand;
    bindCubemapCommand.instanceID  = cubemapInstanceID;
    bindCubemapCommand.uniformName = uniformName;

    RenderCommand command;
    command.type        = RenderCommandType::bindCubemap;
    command.bindCubemap = bindCubemapCommand;

    m_frontBuffer.push_back(command);
}

void Frontend::bindMaterial(renderID materialRenderID)
{
    BindMaterialCommand bindMaterialCommand;
    bindMaterialCommand.materialRenderID = materialRenderID;

    RenderCommand command;
    command.type         = RenderCommandType::bindMaterial;
    command.bindMaterial = bindMaterialCommand;

    m_frontBuffer.push_back(command);
}

void Frontend::changeUsedProgram(ProgramType program)
{
    SetActiveProgramCommand setActiveProgramCommand;
    setActiveProgramCommand.program = program;
    RenderCommand progCommand;
    progCommand.type             = RenderCommandType::setActiveProgram;
    progCommand.setActiveProgram = setActiveProgramCommand;

    m_frontBuffer.push_back(progCommand);
    setProjectionMatrix(m_projMat);
    setViewMatrix(m_viewMat);
}

void Frontend::setCommandBuffer(std::vector<RenderCommand>& newBuffer)
{
    m_frontBuffer = newBuffer;
}

} // namespace Galaxy
