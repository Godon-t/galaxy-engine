#pragma once

#include "Backend.hpp"
#include "frontend/Frontend.hpp"
#include "LightManager.hpp"

#include "data/Transform.hpp"
#include "nodes/Node.hpp"
#include "types/Render.hpp"

#include <optional>

namespace Galaxy {
enum FilterEnum {
    IRRADIANCE
};

class Renderer {
public:
    static Renderer& getInstance();
    void init();
    void shutdown();
    
    void passShadow();
    void addMainCameraDevice(std::shared_ptr<Camera> camera);
    void passPostProcessing(std::shared_ptr<Camera> camera);
    void updateGI();
    void renderFrame();

    inline int getDrawCallsCount() { return m_drawCount; }

    void addObjectToScene(GeometryHandle geometry, std::optional<MaterialHandle> material, const Transform& transform);


    Backend& getBackend(){return m_backend;}
    Frontend& getFrontend(){return m_frontend;}
    LightManager& getLightManager(){return m_lightManager;}

    inline vec2 getRenderingWindowSize() const {return m_mainViewportSize;}

    // TODO: shouldn't be able to retrieve GPU id outside of backend
    inline unsigned int getFrameBufferTextureID(FramebufferHandle framebuffer) { return m_backend.getFrameBufferTextureID(framebuffer); }
    inline unsigned int getRawSceneTextureID() { return m_backend.getFrameBufferTextureID(m_sceneFramebuffer); }
    inline unsigned int getPostProcSceneTextureID() { return m_backend.getFrameBufferTextureID(m_postProcessingFramebuffer); }

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

    FramebufferHandle m_sceneFramebuffer;
    FramebufferHandle m_postProcessingFramebuffer;
    GeometryHandle m_postProcessingQuad;
    vec2 m_mainViewportSize;

    int m_drawCount = 0;
};
}
