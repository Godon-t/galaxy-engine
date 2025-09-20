#pragma once

#include "Backend.hpp"
#include "Frontend.hpp"

#include "data/Transform.hpp"
#include "nodes/Node.hpp"
#include "types/Render.hpp"

namespace Galaxy {
class Renderer {
public:
    static Renderer& getInstance();

    inline void changeUsedProgram(ProgramType prog) { m_frontend.changeUsedProgram(prog); }

    inline void setProjectionMatrix(math::mat4& projection) { m_frontend.setProjectionMatrix(projection); }

    void beginSceneRender(mat4& camTransform);
    void beginSceneRender(const vec3& camPosition, const vec3& camDirection, const vec3& camUp);
    inline void submit(renderID meshID, const Transform& transform) { m_frontend.submit(meshID, transform); }

    void endSceneRender();
    void renderFrame();

    inline renderID instanciateMesh(std::vector<Vertex>& vertices, std::vector<short unsigned int>& indices) { return m_backend.instanciateMesh(vertices, indices); }
    inline renderID instanciateMesh(ResourceHandle<Mesh> mesh, int surfaceIdx = 0) { return m_backend.instanciateMesh(mesh, surfaceIdx); }
    inline void clearMesh(renderID meshID) { m_backend.clearMesh(meshID); }

    inline renderID instanciateTexture(ResourceHandle<Image> image) { return m_backend.instanciateTexture(image); }
    inline void bindTexture(renderID textureInstanceID, char* uniformName) { m_frontend.bindTexture(textureInstanceID, uniformName); }
    inline void clearTexture(renderID textureID) { m_backend.clearTexture(textureID); }

    renderID instanciateMaterial(ResourceHandle<Material> material);
    inline void bindMaterial(renderID materialID) { m_frontend.bindMaterial(materialID); }
    inline void clearMaterial(renderID materialID) { m_backend.clearMaterial(materialID); }
    void updateMaterial(renderID materialID, ResourceHandle<Material> material);

    inline renderID generateCube(float dimmension, bool inward, std::function<void()> destroyCallback) { return m_backend.generateCube(dimmension, inward, destroyCallback); }
    inline renderID generateQuad(vec2 dimmensions, std::function<void()> destroyCallback) { return m_backend.generateQuad(dimmensions, destroyCallback); }

    inline renderID instanciateCubemap(std::array<ResourceHandle<Image>, 6> faces) { return m_backend.instanciateCubemap(faces); }
    inline renderID instanciateCubemap() { return m_backend.instanciateCubemap(); }
    inline void clearCubemap(renderID cubemapID) { m_backend.clearCubemap(cubemapID); }
    inline void bindCubemap(renderID cubemapInstanceID, char* uniformName) { return m_frontend.bindCubemap(cubemapInstanceID, uniformName); }

    inline void submitPBR(renderID meshID, renderID materialID, const Transform& transform) { m_frontend.submitPBR(meshID, materialID, transform); }
    void renderFromPoint(vec3 position, Node& root, renderID targetCubemapID);

    inline int getDrawCallsCount() { return m_drawCount; }

private:
    Renderer();
    ~Renderer();

    void switchCommandBuffer();

    // TODO: double buffering of commands not used currently (no multithreading)
    std::vector<std::vector<RenderCommand>> m_commandBuffers;
    int m_frontCommandBufferIdx;

    Frontend m_frontend;
    Backend m_backend;

    int m_drawCount = 0;
};
}
