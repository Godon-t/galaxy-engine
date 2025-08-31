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
}

void Frontend::setProjectionMatrix(math::mat4& projection)
{
    SetProjectionCommand setProjectionCommand;
    setProjectionCommand.projection = projection;

    RenderCommand command;
    command.type          = RenderCommandType::setProjection;
    command.setProjection = setProjectionCommand;

    m_frontBuffer.push_back(command);
}

void Frontend::setCommandBuffer(std::vector<RenderCommand>& newBuffer)
{
    m_frontBuffer = newBuffer;
}

} // namespace Galaxy
