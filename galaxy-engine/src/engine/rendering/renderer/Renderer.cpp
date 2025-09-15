#include "Renderer.hpp"

#include "pch.hpp"

#include "Core.hpp"
#include "rendering/CameraManager.hpp"
#include "rendering/GPUInstances/FrameBuffer.hpp"
#include "rendering/OpenglHelper.hpp"

namespace Galaxy {
Renderer::Renderer()
    : m_commandBuffers(2)
    , m_frontCommandBufferIdx(0)
    , m_frontend(m_commandBuffers[m_frontCommandBufferIdx])
    , m_backend()
{
}

Renderer::~Renderer()
{
}

void Renderer::switchCommandBuffer()
{
    m_frontCommandBufferIdx = 1 - m_frontCommandBufferIdx;
    m_frontend.setCommandBuffer(m_commandBuffers[m_frontCommandBufferIdx]);
}

Renderer& Renderer::getInstance()
{
    static Renderer renderer;
    return renderer;
}

void Renderer::setProjectionMatrix(math::mat4& projection)
{
    m_frontend.setProjectionMatrix(projection);
}

void Renderer::beginSceneRender(mat4& camTransform)
{
    auto clearColor = math::vec4(0.2, 0.2, 0.25, 1.0);
    m_frontend.clear(clearColor);
    auto viewMatrix = CameraManager::processViewMatrix(camTransform);
    m_frontend.setViewMatrix(viewMatrix);
}

void Renderer::beginSceneRender(const vec3& camPosition, const vec3& camDirection, const vec3& camUp)
{
    auto clearColor = math::vec4(0.2, 0.2, 0.25, 1.0);
    m_frontend.clear(clearColor);
    auto viewMatrix = lookAt(camPosition, camDirection, camUp);
    m_frontend.setViewMatrix(viewMatrix);
}

void Renderer::changeUsedProgram(ProgramType prog)
{
    m_frontend.changeUsedProgram(prog);
}

void Renderer::submit(renderID meshID, const Transform& transform)
{
    m_frontend.submit(meshID, transform);
}

void Renderer::bindTexture(renderID textureInstanceID, char* uniformName)
{
    m_frontend.bindTexture(textureInstanceID, uniformName);
}

void Renderer::endSceneRender()
{
}

void Renderer::renderFrame()
{
    m_backend.processCommands(m_commandBuffers[m_frontCommandBufferIdx]);
    m_commandBuffers[m_frontCommandBufferIdx].clear();
    switchCommandBuffer();
}

renderID Renderer::instanciateMesh(std::vector<Vertex>& vertices, std::vector<short unsigned int>& indices)
{
    return m_backend.instanciateMesh(vertices, indices);
}

renderID Renderer::instanciateMesh(ResourceHandle<Mesh> mesh, int surfaceIdx)
{
    return m_backend.instanciateMesh(mesh, surfaceIdx);
}

void Renderer::clearMesh(renderID meshID)
{
    m_backend.clearMesh(meshID);
}
renderID Renderer::instanciateTexture(ResourceHandle<Image> image)
{
    return m_backend.instanciateTexture(image);
}
void Renderer::clearTexture(renderID textureID)
{
    m_backend.clearTexture(textureID);
}

renderID Renderer::generateCube(float dimmension, bool inward, std::function<void()> destroyCallback)
{
    return m_backend.generateCube(dimmension, inward, destroyCallback);
}

renderID Renderer::instanciateCubemap()
{
    return m_backend.instanciateCubemap();
}
renderID Renderer::instanciateCubemap(std::array<ResourceHandle<Image>, 6> faces)
{
    return m_backend.instanciateCubemap(faces);
}
void Renderer::bindCubemap(renderID cubemapInstanceID, char* uniformName)
{
    return m_frontend.bindCubemap(cubemapInstanceID, uniformName);
}

void Renderer::renderFromPoint(vec3 position, Node& root, renderID targetCubemapID)
{
    switchCommandBuffer();
    m_commandBuffers[m_frontCommandBufferIdx].clear();

    vec3 orientations[6], ups[6];
    orientations[0] = { 1, 0, 0 };
    orientations[1] = { -1, 0, 0 };
    ups[0] = ups[1] = { 0, -1, 0 };

    orientations[2] = { 0, 1, 0 };
    orientations[3] = { 0, -1, 0 };
    ups[2]          = { 0, 0, 1 };
    ups[3]          = { 0, 0, -1 };

    orientations[4] = { 0, 0, 1 };
    orientations[5] = { 0, 0, -1 };
    ups[4] = ups[5] = { 0, -1, 0 };

    mat4 projection = perspective(radians(90.0f), 1.f, 0.1f, 100.f);

    Cubemap& targetCubemap = *m_backend.m_cubemapInstances.get(targetCubemapID);
    targetCubemap.useFloat = true;
    targetCubemap.resize(512);
    CubemapFrameBuffer cubemapBuffer(targetCubemap);

    // root.draw();
    for (int i = 0; i < 6; i++) {
        cubemapBuffer.bind(i);
        vec4 color = vec4(1, 0, 0, 1);
        m_frontend.clear(color);
        // beginSceneRender(vec3(0), orientations[1], ups[1]);
        m_backend.processCommands(m_commandBuffers[m_frontCommandBufferIdx]);
    }
    cubemapBuffer.unbind();
    cubemapBuffer.destroy();

    m_commandBuffers[m_frontCommandBufferIdx].clear();
    switchCommandBuffer();
}

renderID Renderer::instanciateMaterial(ResourceHandle<Material> material)
{
    return m_backend.instanciateMaterial(material);
}
void Renderer::bindMaterial(renderID materialID)
{
    m_frontend.bindMaterial(materialID);
}
void Renderer::clearMaterial(renderID materialID)
{
    m_backend.clearMaterial(materialID);
}
}
