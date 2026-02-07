#pragma once

#include "pch.hpp"

#include "SceneContext.hpp"
#include "rendering/renderer/RenderCommand.hpp"
#include "data/Transform.hpp"
#include "types/Render.hpp"
#include "RenderDevice.hpp"

#include "queue"

namespace Galaxy {
class Frontend {
public:
    void addRenderDevice(RenderDevice& renderDevice){m_frameDevices.push_back(renderDevice);}
    void processDevices(); 


    Frontend(std::vector<RenderCommand>* commandBuffer);

    // void storeCanvaResult(std::string& path);

    void submit(renderID meshID);
    void submit(renderID meshID, const Transform& transform);
    void clear(vec4& color);

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
    void setUniform(std::string uniformName, ivec3 value);
    void setUniform(std::string uniformName, vec2 value);
    template <typename T>
    void updateUniform(renderID id, const T& payload)
    {
        auto updateCommand = UpdateUBOCommand::make(id, payload);
        pushCommand(updateCommand);
    }
    void bindUBO(renderID id, unsigned int idx);

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


    inline void removeMaterialID(renderID matID) { m_frameContext.removeMaterialID(matID); }
    void notifyMaterialUpdated(renderID materialID, bool isTransparent);
    inline void clearContext(){m_frameContext.clear();}
    
private:
    // TODO: Create cameraFrustum object
    void dumpCommandsToBuffer(vec3& cameraPosition);
    
    void setViewMatrix(const math::mat4& view);
    void setProjectionMatrix(const math::mat4& projection);
    void pushCommand(RenderCommand command);
    void saveFrameBuffer(renderID framebufferID, std::string& path);


    SceneContext m_frameContext;
    std::vector<RenderDevice> m_frameDevices;

    std::vector<RenderCommand>* m_frontBuffer;

    FramebufferTextureFormat m_currentFramebufferFormat = FramebufferTextureFormat::None;
};
} // namespace Galaxy
