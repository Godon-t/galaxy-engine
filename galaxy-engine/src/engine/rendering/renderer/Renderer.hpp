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
    void addMainCameraDevice(std::shared_ptr<Camera> camera);
    void passPostProcessing(std::shared_ptr<Camera> camera);
    void updateGI();
    void renderFrame();

    inline int getDrawCallsCount() { return m_drawCount; }

    void addObjectToScene(renderID meshID, renderID materialID, const Transform& transform);


    Backend& getBackend(){return m_backend;}
    Frontend& getFrontend(){return m_frontend;}
    LightManager& getLightManager(){return m_lightManager;}

    inline vec2 getRenderingWindowSize() const {return m_mainViewportSize;}

    // TODO: shouldn't be able to retrieve GPU id outside of backend
    inline unsigned int getFrameBufferTextureID(renderID frameBufferID) { return m_backend.getFrameBufferTextureID(frameBufferID); }
    inline unsigned int getRawSceneTextureID() { return m_backend.getFrameBufferTextureID(m_sceneFrameBufferID); }
    inline unsigned int getPostProcSceneTextureID() { return m_backend.getFrameBufferTextureID(m_postProcessingBufferID); }

    // TODO: Resizing unbind framebuffer
    void resize(unsigned int width, unsigned int height);

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
    renderID m_postProcessingBufferID;
    renderID m_postProcessingQuadID;
    vec2 m_mainViewportSize;

    int m_drawCount = 0;
};
}