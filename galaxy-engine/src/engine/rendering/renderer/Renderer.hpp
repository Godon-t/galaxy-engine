#pragma once

#include "Backend.hpp"
#include "Frontend.hpp"
#include "LightManager.hpp"

#include "data/Transform.hpp"
#include "nodes/Node.hpp"
#include "types/Render.hpp"

namespace Galaxy {
enum FilterEnum {
    IRRADIANCE
};

class Renderer {
public:
    static Renderer& getInstance();
    void init();

    inline void changeUsedProgram(ProgramType prog) { m_frontend.changeUsedProgram(prog); }

    // inline void setProjectionMatrix(math::mat4& projection) { m_frontend.setProjectionMatrix(projection); }

    void beginSceneRender(const mat4& camTransform, const vec2& dimmensions);
    void beginSceneRender(const vec3& camPosition, const vec3& camDirection, const vec3& camUp, const vec2& dimmensions);
    inline void beginCanvaNoBuffer() { m_frontend.beginCanvaNoBuffer(); }
    void beginCanva(const mat4& camTransform, const vec2& dimmensions, renderID framebufferID, FramebufferTextureFormat framebufferFormat, int cubemapIdx = -1);
    inline void beginCanva(const mat4& viewMat, const mat4& projectionMat, renderID framebufferID, FramebufferTextureFormat framebufferFormat, int cubemapIdx = -1)
    {
        m_frontend.beginCanva(viewMat, projectionMat, framebufferID, framebufferFormat, cubemapIdx);
    }
    inline void endCanva() { m_frontend.endCanva(); }
    inline void linkCanvaColorToTexture(renderID textureID) { m_frontend.linkCanvaColorToTexture(textureID); }
    inline void linkCanvaDepthToTexture(renderID textureID) { m_frontend.linkCanvaDepthToTexture(textureID); }

    void endSceneRender();

    void shadowPass();

    void renderFrame();

    inline void submit(renderID meshID, const Transform& transform) { m_frontend.submit(meshID, transform); }

    inline renderID instanciateMesh(std::vector<Vertex>& vertices, std::vector<short unsigned int>& indices) { return m_backend.instanciateMesh(vertices, indices); }
    inline renderID instanciateMesh(ResourceHandle<Mesh> mesh, int surfaceIdx = 0) { return m_backend.instanciateMesh(mesh, surfaceIdx); }
    inline void clearMesh(renderID meshID) { m_backend.clearMesh(meshID); }

    inline renderID instanciateTexture(ResourceHandle<Image> image) { return m_backend.instantiateTexture(image); }
    inline void bindTexture(renderID textureInstanceID, char* uniformName) { m_frontend.bindTexture(textureInstanceID, uniformName); }
    inline void clearTexture(renderID textureID) { m_backend.clearTexture(textureID); }

    renderID instanciateMaterial(ResourceHandle<Material> material);
    inline void bindMaterial(renderID materialID) { m_frontend.bindMaterial(materialID); }
    inline void clearMaterial(renderID materialID) { m_backend.clearMaterial(materialID); }
    void updateMaterial(renderID materialID, ResourceHandle<Material> material);

    inline renderID generateCube(float dimmension, bool inward, std::function<void()> destroyCallback) { return m_backend.generateCube(dimmension, inward, destroyCallback); }
    inline renderID generateQuad(vec2 dimmensions, std::function<void()> destroyCallback) { return m_backend.generateQuad(dimmensions, destroyCallback); }
    inline renderID generatePyramid(float baseSize, float height, std::function<void()> destroyCallback) { return m_backend.generatePyramid(baseSize, height, destroyCallback); }

    inline renderID instantiateTexture() { return m_backend.instantiateTexture(); }
    inline renderID instanciateCubemap(std::array<ResourceHandle<Image>, 6> faces) { return m_backend.instanciateCubemap(faces); }
    inline renderID instanciateCubemap() { return m_backend.instanciateCubemap(); }
    inline void clearCubemap(renderID cubemapID) { m_backend.clearCubemap(cubemapID); }
    inline void useCubemap(renderID cubemapInstanceID, char* uniformName) { return m_frontend.useCubemap(cubemapInstanceID, uniformName); }

    inline renderID instanciateFrameBuffer(unsigned int width, unsigned int height, FramebufferTextureFormat format) { return m_backend.instanciateFrameBuffer(width, height, format); }
    inline void clearFrameBuffer(renderID frameBufferID) { m_backend.clearFrameBuffer(frameBufferID); }
    inline void bindFrameBuffer(renderID frameBufferInstanceID) { m_frontend.bindFrameBuffer(frameBufferInstanceID); }
    inline void unbindFrameBuffer(renderID frameBufferInstanceID) { m_frontend.unbindFrameBuffer(frameBufferInstanceID); }
    inline void resizeFrameBuffer(renderID frameBufferID, unsigned int width, unsigned int height) { m_backend.resizeFrameBuffer(frameBufferID, width, height); }
    inline void resizeCubemapFrameBuffer(renderID framebufferID, unsigned int size) { m_backend.resizeCubemapFrameBuffer(framebufferID, size); }
    inline void resizeCubemap(renderID targetID, unsigned int size) { m_frontend.updateCubemap(targetID, size); }
    inline FramebufferTextureFormat getFramebufferFormat(renderID framebufferID) { return m_backend.getFramebufferFormat(framebufferID); }

    inline void debugMessage(std::string message) { m_frontend.addDebugMsg(message); }

    inline void attachTextureToColorFramebuffer(renderID textureID, renderID framebufferID) { m_frontend.attachTextureToColorFramebuffer(textureID, framebufferID); }
    inline void attachTextureToDepthFramebuffer(renderID textureID, renderID framebufferID) { m_frontend.attachTextureToDepthFramebuffer(textureID, framebufferID); }

    inline void setUniform(std::string uniformName, bool value) { m_frontend.setUniform(uniformName, value); }
    inline void setUniform(std::string uniformName, mat4 value) { m_frontend.setUniform(uniformName, value); }
    inline void setUnicolorObjectColor(const vec3& color) { m_frontend.setUnicolorObjectColor(color); }

    // TODO: Resizing unbind framebuffer
    void resize(unsigned int width, unsigned int height)
    {
        m_backend.resizeFrameBuffer(m_sceneFrameBufferID, width, height);
        m_backend.resizeFrameBuffer(m_postProcessingBufferID, width, height);
    }

    inline void submitPBR(renderID meshID, renderID materialID, const Transform& transform) { m_frontend.submitPBR(meshID, materialID, transform); }
    void renderFromPoint(vec3 position, Node& root, renderID targetCubemapID);
    void applyFilterOnCubemap(renderID skyboxMesh, renderID sourceID, renderID targetID, FilterEnum filter);

    inline int getDrawCallsCount() { return m_drawCount; }

    // TODO: remove ASAP
    inline unsigned int getFrameBufferTextureID(renderID frameBufferID) { return m_backend.getFrameBufferTextureID(frameBufferID); }
    inline unsigned int getRawSceneTextureID() { return m_backend.getFrameBufferTextureID(m_sceneFrameBufferID); }
    inline unsigned int getPostProcSceneTextureID() { return m_backend.getFrameBufferTextureID(m_postProcessingBufferID); }

    inline LightManager& getLightManager() { return m_lightManager; }

private:
    Renderer();
    ~Renderer();

    void switchCommandBuffer();
    void applyPostProcessing();
    // TODO: double buffering of commands not used currently (no multithreading)
    std::vector<std::vector<RenderCommand>> m_commandBuffers;
    int m_frontCommandBufferIdx;

    Frontend m_frontend;
    Backend m_backend;
    LightManager m_lightManager;

    renderID m_sceneFrameBufferID;
    renderID m_postProcessingBufferID;
    renderID m_postProcessingQuadID;
    renderID m_cubemapFramebufferID;

    mat4 m_currentView;
    mat4 m_currentProj;

    renderID m_testRect;
    renderID m_testRectText;
    renderID m_testRectFB;

    vec3 m_cubemap_orientations[6], m_cubemap_ups[6];

    int m_drawCount = 0;
};
}
