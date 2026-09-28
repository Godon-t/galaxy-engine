#pragma once

#include "Backend.hpp"

#include "data/Transform.hpp"
#include "nodes/Node.hpp"
#include "types/Render.hpp"
#include "rendering/CameraManager.hpp"
#include "LightManager.hpp"
#include "frontend/Frontend.hpp"

#include <optional>

namespace Galaxy {
enum FilterEnum {
    IRRADIANCE
};

class Frontend;

class Renderer {
public:
    static Renderer& getInstance();
    void init();
    void shutdown();
    
    void addMainCameraDevice(std::shared_ptr<Camera> camera);
    void updateGI();
    void renderFrame();
    [[nodiscard]] std::size_t getDrawCallsCount() const noexcept { return m_backend.getDrawCallsCount(); }

    void addObjectToScene(GeometryHandle geometry, std::optional<MaterialHandle> material, const Transform& transform);

    Backend& getBackend(){return m_backend;}
    Frontend& getFrontend(){return m_frontend;}
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
    Frontend m_frontend;

    FramebufferHandle m_sceneFramebuffer;
    vec2 m_mainViewportSize;
};
}
