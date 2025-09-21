#pragma once

#include "pch.hpp"

#include "RenderCommand.hpp"
#include "data/Transform.hpp"
#include "types/Render.hpp"

#include "queue"

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
    void bindFrameBuffer(renderID frameBufferInstanceID);
    void unbindFrameBuffer(renderID frameBufferInstanceID);
    void bindMaterial(renderID materialRenderID);
    void changeUsedProgram(ProgramType program);

    void submitPBR(renderID meshID, renderID materialID, const Transform& transform);

    void dumpCommandsToBuffer();

    void setCommandBuffer(std::vector<RenderCommand>* newBuffer);

    inline void setTransparency(renderID matID, bool state) { m_materialsTransparency[matID] = state; }
    inline void removeMaterialID(renderID matID)
    {
        m_materialsTransparency.erase(matID);
        m_materialToSubmitCommand.erase(matID);
    }

private:
    struct DistCompare {
        static vec3 camPosition;
        bool operator()(const std::pair<renderID, RenderCommand>& a, const std::pair<renderID, RenderCommand>& b) const;
    };

    std::unordered_map<renderID, bool> m_materialsTransparency;
    std::unordered_map<renderID, std::vector<RenderCommand>> m_materialToSubmitCommand;

    std::vector<RenderCommand>* m_frontBuffer;

    mat4 m_projMat;
    mat4 m_viewMat;
};
} // namespace Galaxy
