#pragma once

#include "pch.hpp"

#include "RenderCommand.hpp"
#include "data/Transform.hpp"
#include "types/Render.hpp"

namespace Galaxy {
class Frontend {
public:
    Frontend(std::vector<RenderCommand>* commandBuffer);

    void submit(renderID meshID, const Transform& transform);
    void clear(math::vec4& color);
    void setViewMatrix(math::mat4& view);
    void setProjectionMatrix(math::mat4& projection);
    void bindTexture(renderID textureInstanceID, char* uniformName);
    void bindCubemap(renderID cubemapInstanceID, char* uniformName);
    void bindMaterial(renderID materialRenderID);
    void changeUsedProgram(ProgramType program);

    void setCommandBuffer(std::vector<RenderCommand>* newBuffer);

private:
    std::vector<RenderCommand>* m_frontBuffer;

    mat4 m_projMat;
    mat4 m_viewMat;
};
} // namespace Galaxy
