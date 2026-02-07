#pragma once

#include "Backend.hpp"
#include "frontend/Frontend.hpp"
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
    
    void passShadow();
    void addMainCameraDevice(const mat4& camTransform);
    void passPostProcessing();
    void updateGI();
    void renderFrame();

    inline int getDrawCallsCount() { return m_drawCount; }


    Backend& getBackend(){return m_backend;}
    Frontend& getFrontend(){return m_frontend;}
    LightManager& getLightManager(){return m_lightManager;}

    inline vec2 getRenderingWindowSize() const {return m_mainViewportSize;}

    // TODO: shouldn't be able to retrieve GPU id outside of backend
    inline unsigned int getFrameBufferTextureID(renderID frameBufferID) { return m_backend.getFrameBufferTextureID(frameBufferID); }
    inline unsigned int getRawSceneTextureID() { return m_backend.getFrameBufferTextureID(m_sceneFrameBufferID); }
    inline unsigned int getPostProcSceneTextureID() { return getRawSceneTextureID(); }// { return m_backend.getFrameBufferTextureID(m_postProcessingBufferID); }

    // TODO: Resizing unbind framebuffer
    void resize(unsigned int width, unsigned int height)
    {
        m_backend.resizeFrameBuffer(m_sceneFrameBufferID, width, height);
        // m_backend.resizeFrameBuffer(m_postProcessingBufferID, width, height);
        m_mainViewportSize.x = width;
        m_mainViewportSize.y = height;
    }

private:
    Renderer();
    ~Renderer();

    void switchCommandBuffer();
    void applyPostProcessing();

    std::vector<std::vector<RenderCommand>> m_commandBuffers;
    int m_frontCommandBufferIdx;


    Frontend m_frontend;
    Backend m_backend;
    LightManager m_lightManager;

    renderID m_sceneFrameBufferID;
    vec2 m_mainViewportSize;

    int m_drawCount = 0;
};
}