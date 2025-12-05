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

    void beginCanvaNoBuffer();
    void beginCanva(const mat4& viewMat, const mat4& projectionMat, renderID framebufferID, FramebufferTextureFormat framebufferFormat, int cubemapIdx = -1);
    void endCanva();
    void processCanvas();
    void linkCanvaColorToTexture(renderID textureID);
    void linkCanvaDepthToTexture(renderID textureID);
    void storeCanvaResult(std::string& path);
    void avoidCanvaBufferClear();

    void submit(renderID meshID);
    void submit(renderID meshID, const Transform& transform);
    void clear(math::vec4& color);

    mat4 getProjectionMatrix();

    void bindTexture(renderID textureInstanceID, char* uniformName);
    void attachTextureToColorFramebuffer(renderID textureID, renderID framebufferID, int attachmentIdx);
    void attachTextureToDepthFramebuffer(renderID textureID, renderID framebufferID);
    void attachCubemapToFramebuffer(renderID cubemapID, renderID framebufferID, int colorIdx = 0);
    void useCubemap(renderID cubemapInstanceID, char* uniformName);
    void bindFrameBuffer(renderID frameBufferInstanceID, int cubemapFaceIdx = -1);
    void unbindFrameBuffer(renderID frameBufferInstanceID, bool cubemap = false);
    void bindMaterial(renderID materialRenderID);
    // TODO: rename to match setActiveProgram command
    void changeUsedProgram(ProgramType program);
    void initPostProcessing(renderID frameBufferID);

    void setUniform(std::string uniformName, bool value);
    void setUniform(std::string uniformName, float value);
    void setUniform(std::string uniformName, int value);
    void setUniform(std::string uniformName, mat4 value);
    void setUniform(std::string uniformName, vec3 value);
    void setUniform(std::string uniformName, vec2 value);

    void setFramebufferAsTextureUniform(renderID framebufferID, std::string uniformName, int textureIdx);

    void setViewport(vec2 position, vec2 dimmension);
    void resizeTexture(renderID textureID, unsigned int width, unsigned int height);
    void setTextureFormat(renderID textureID, TextureFormat format);
    void updateCubemap(renderID targetID, unsigned int resolution);

    void addDebugMsg(std::string message);
    void submitDebugLine(vec3 start, vec3 end, vec3 color);
    void drawDebug();

    void submitPBR(renderID meshID, renderID materialID, const Transform& transform);

    void setCommandBuffer(std::vector<RenderCommand>* newBuffer);

    inline void setTransparency(renderID matID, bool state) { m_materialsTransparency[matID] = state; }
    inline void removeMaterialID(renderID matID)
    {
        m_materialsTransparency.erase(matID);
        // m_materialToSubmitCommand.erase(matID);
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
    void saveFrameBuffer(renderID framebufferID, std::string& path);

    std::unordered_map<renderID, bool> m_materialsTransparency;

    std::vector<RenderCanva> m_canvas;
    size_t m_currentCanvaIdx;

    std::vector<RenderCommand>* m_frontBuffer;

    mat4 m_projMat;
    mat4 m_viewMat;
    FramebufferTextureFormat m_currentFramebufferFormat = FramebufferTextureFormat::None;
};
} // namespace Galaxy
