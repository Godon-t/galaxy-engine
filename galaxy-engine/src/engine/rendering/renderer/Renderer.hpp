#pragma once

#include "Backend.hpp"
#include "Frontend.hpp"

#include "data/Transform.hpp"
#include "types/Render.hpp"

namespace Galaxy {
class Renderer {
public:
    static Renderer& getInstance();

    void beginSceneRender(mat4& camTransform);
    void submit(renderID meshID, const Transform& transform);
    void bindTexture(renderID textureInstanceID, char* uniformName);

    void endSceneRender();
    void renderFrame();

    renderID instanciateMesh(std::vector<Vertex>& vertices, std::vector<short unsigned int>& indices);
    renderID instanciateMesh(ResourceHandle<Mesh> mesh, int surfaceIdx = 0);
    void clearMesh(renderID meshID);

    renderID instanciateTexture(ResourceHandle<Image> image);
    void clearTexture(renderID textureID);

private:
    Renderer();
    ~Renderer();

    // TODO: double buffering of commands not used currently (no multithreading)
    std::vector<std::vector<RenderCommand>> m_commandBuffers;
    int m_frontCommandBufferIdx;

    Frontend m_frontend;
    Backend m_backend;
};
}
