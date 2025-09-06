#include "Backend.hpp"

#include "Helper.hpp"
#include "Log.hpp"
#include "gl_headers.hpp"
#include "rendering/OpenglHelper.hpp"

namespace Galaxy {
Backend::Backend(size_t maxSize)
    : m_visualInstances(maxSize)
{
    GLenum error = glGetError();

    checkOpenGLErrors("error before glewInit");
    glewExperimental    = true; // Needed for core profile
    int glewInitialized = glewInit();
    GLX_CORE_ASSERT(glewInitialized == GLEW_OK, "Failed to initialize GLEW")

    checkOpenGLErrors("known error after glewInit");

    glClearColor(1.f, 0.f, 0.2f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    // glDepthFunc(GL_LEQUAL);
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // glEnable(GL_CULL_FACE);
    glDisable(GL_CULL_FACE);

    m_mainProgram    = std::move(Program(engineRes("shaders/test.glsl")));
    m_textureProgram = std::move(Program(engineRes("shaders/texture.glsl")));

    checkOpenGLErrors("Renderer constructor");
}

renderID Backend::instanciateMesh(std::vector<Vertex>& vertices, std::vector<short unsigned int>& indices, std::function<void()> destroyCallback)
{
    if (!m_visualInstances.canAddInstance())
        return -1;

    renderID meshID = m_visualInstances.createResourceInstance();
    m_visualInstances.get(meshID)->init(vertices, indices);

    if (destroyCallback) {
        m_gpuDestroyNotifications[meshID] = destroyCallback;
    }

    return meshID;
}

renderID Backend::instanciateMesh(ResourceHandle<Mesh> mesh, int surfaceIdx)
{
    renderID subMeshID = mesh.getResource().getVisualID(surfaceIdx);
    if (subMeshID != 0) {
        m_visualInstances.increaseCount(subMeshID);
        return subMeshID;
    }

    renderID visualID = m_visualInstances.createResourceInstance();

    mesh.getResource().onLoaded([this, mesh, visualID, surfaceIdx] {
        const auto& meshRes = mesh.getResource();
        m_visualInstances.get(visualID)->init(
            meshRes.getVertices(surfaceIdx),
            meshRes.getIndices(surfaceIdx));
    });

    m_gpuDestroyNotifications[visualID] = [surfaceIdx, mesh] mutable { mesh.getResource().notifyGpuInstanceDestroyed(surfaceIdx); };

    return visualID;
}

void Backend::clearMesh(renderID meshID)
{
    if (!m_visualInstances.tryRemove(meshID))
        return;

    auto it = m_gpuDestroyNotifications.find(meshID);
    if (it != m_gpuDestroyNotifications.end()) {
        it->second();
        m_gpuDestroyNotifications.erase(meshID);
    }
}

renderID Backend::instanciateTexture(ResourceHandle<Image> image)
{
    renderID existingID = image.getResource().getTextureID();
    if (existingID != 0) {
        m_textureInstances.increaseCount(existingID);
        return existingID;
    }

    renderID textureID = m_textureInstances.createResourceInstance();

    image.getResource().onLoaded([this, image, textureID] {
        const auto& imgRes = image.getResource();
        m_textureInstances.get(textureID)->init(imgRes.getData(), imgRes.getWidth(), imgRes.getHeight(), imgRes.getNbChannels());
    });

    m_gpuDestroyNotifications[textureID] = [image] mutable { image.getResource().notifyGpuInstanceDestroyed(); };

    return textureID;
}

void Backend::clearTexture(renderID textureID)
{
    if (!m_textureInstances.tryRemove(textureID))
        return;

    auto it = m_gpuDestroyNotifications.find(textureID);
    if (it != m_gpuDestroyNotifications.end()) {
        it->second();
        m_gpuDestroyNotifications.erase(textureID);
    }
}

void Backend::processCommands(std::vector<RenderCommand>& commands)
{
    for (auto& command : commands) {
        processCommand(command);
    }

    Texture::resetActivationInts();
}

void Backend::processCommand(RenderCommand& command)
{
    switch (command.type) {
    case RenderCommandType::setActiveProgram: {
        if (command.setActiveProgram.program == BaseProgramEnum::TEXTURE)
            m_activeProgram = &m_textureProgram;
        else if (command.setActiveProgram.program == BaseProgramEnum::PBR)
            m_activeProgram = &m_mainProgram;
        else
            GLX_CORE_ASSERT(false, "unknown asked program!");

        m_activeProgram->use();
        break;
    }
    case RenderCommandType::clear: {
        auto& clearColor = command.clear.color;
        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        break;
    }
    case RenderCommandType::setView:
        m_activeProgram->updateViewMatrix(command.setView.view);
        break;
    case RenderCommandType::setProjection:
        m_activeProgram->updateProjectionMatrix(command.setProjection.projection);
        break;
    case RenderCommandType::draw: {
        auto& modelMatrix = command.draw.model;
        m_activeProgram->updateModelMatrix(modelMatrix);
        m_visualInstances.get(command.draw.instanceId)->draw();
        break;
    }
    case RenderCommandType::bindTexture: {
        auto uniLoc = glGetUniformLocation(m_activeProgram->getProgramID(), command.bindTexture.uniformName);
        m_textureInstances.get(command.bindTexture.instanceID)->activate(uniLoc);
        checkOpenGLErrors("Bind texture");
        break;
    }
    default:
        GLX_CORE_ERROR("Unknown render command");
        break;
    }
}

} // namespace Galaxy