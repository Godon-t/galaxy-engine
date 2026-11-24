#pragma once

#include "backend/Backend.hpp"

#include "data/Transform.hpp"
#include "nodes/Node.hpp"
#include "types/Render.hpp"
#include "rendering/frontend/CameraManager.hpp"
#include "frontend/LightManager.hpp"
#include "frontend/Frontend.hpp"

#include <optional>

namespace Galaxy {

class Frontend;

class Renderer {
public:
    static Renderer& getInstance();
    void init();
    void shutdown();
    
    void addMainCamera(const Camera& camera);
    void updateGI();
    void renderFrame();
    [[nodiscard]] std::size_t getDrawCallsCount() const noexcept { return m_backend.getDrawCallsCount(); }

    Backend& getBackend(){return m_backend;}
    Frontend& getFrontend(){return m_frontend;}
    RenderGraph& getRenderGraph(){return m_graph;}
    LightManager& getLightManager(){return m_frontend.getLightManager();}

    inline vec2 getRenderingWindowSize() const {return m_mainViewportSize;}

    // TODO: shouldn't be able to retrieve GPU id outside of backend
    inline unsigned int getSceneTextureID() { return m_backend.getFrameBufferTextureID(m_sceneFramebuffer); }

    // TODO: Resizing unbind framebuffer
    void resize(unsigned int width, unsigned int height);

private:
    Renderer();
    ~Renderer();

    Backend m_backend;
    RenderGraph m_graph;
    Frontend m_frontend;


    FramebufferHandle m_sceneFramebuffer;
    vec2 m_mainViewportSize;
};
}
