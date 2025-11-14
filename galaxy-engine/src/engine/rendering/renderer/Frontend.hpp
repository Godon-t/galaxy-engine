#pragma once

#include "pch.hpp"

#include "RenderCanva.hpp"
#include "RenderCommand.hpp"
#include "data/Transform.hpp"
#include "types/Render.hpp"

#include "queue"

namespace Galaxy {
class Frontend {
public:
    Frontend(std::vector<RenderCommand>* commandBuffer);

    void beginCanva(const mat4& viewMat, const mat4& projectionMat, renderID framebufferID, FramebufferTextureFormat framebufferFormat, int cubemapIdx = -1);
    void endCanva();
    void attachCurrentCanva(renderID targetTextureID);
    void processCanvas();

    void submit(renderID meshID);
    void submit(renderID meshID, const Transform& transform);
    void clear(math::vec4& color);

    mat4 getProjectionMatrix();

    void bindTexture(renderID textureInstanceID, char* uniformName);
    void attachTextureToColorFramebuffer(renderID textureID, renderID framebufferID);
    void attachTextureToDepthFramebuffer(renderID textureID, renderID framebufferID);
    void bindCubemap(renderID cubemapInstanceID, char* uniformName);
    void bindFrameBuffer(renderID frameBufferInstanceID, int cubemapFaceIdx = -1);
    void unbindFrameBuffer(renderID frameBufferInstanceID, bool cubemap = false);
    void bindMaterial(renderID materialRenderID);
    // TODO: rename to match setActiveProgram command
    void changeUsedProgram(ProgramType program);
    void initPostProcessing(renderID frameBufferID);
    void setUniform(char* uniformName, bool value);

    void submitPBR(renderID meshID, renderID materialID, const Transform& transform);

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

    void dumpCommandsToBuffer(RenderCanva& canva);
    void setViewMatrix(const math::mat4& view);
    void setProjectionMatrix(const math::mat4& projection);
    void pushCommand(RenderCommand command);

    std::unordered_map<renderID, bool> m_materialsTransparency;

    std::vector<RenderCanva> m_canvas;
    size_t m_currentCanvaIdx;

    std::unordered_map<renderID, std::vector<RenderCommand>> m_materialToSubmitCommand;

    std::vector<RenderCommand>* m_frontBuffer;

    mat4 m_projMat;
    mat4 m_viewMat;
    FramebufferTextureFormat m_currentFramebufferFormat = FramebufferTextureFormat::None;
};
} // namespace Galaxy
