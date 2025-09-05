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

renderID Backend::instanciateMesh(std::vector<Vertex>& vertices, std::vector<short unsigned int>& indices)
{
    if (!m_visualInstances.canAddInstance())
        return -1;

    renderID meshID = m_visualInstances.createResourceInstance();
    m_visualInstances.get(meshID)->init(vertices, indices);

    return meshID;
}

renderID Backend::instanciateMesh(ResourceHandle<Mesh> mesh, int surfaceIdx)
{
    size_t handleID    = reinterpret_cast<size_t>(&mesh.getResource());
    auto it            = m_meshResourceTable.find(handleID);
    bool resourceExist = it != m_meshResourceTable.end();
    if (resourceExist) {
        auto& activesSubMeshes = it->second.activesSubMeshes;
        auto subMeshIt         = activesSubMeshes.find(surfaceIdx);
        if (subMeshIt != activesSubMeshes.end()) {
            renderID subMeshID = activesSubMeshes[surfaceIdx];
            m_visualInstances.increaseCount(subMeshID);
            return subMeshID;
        }
    }

    renderID visualID = m_visualInstances.createResourceInstance();

    if (!resourceExist) {
        MeshHandle meshHandle;
        meshHandle.mesh               = mesh;
        m_meshResourceTable[handleID] = meshHandle;
    }
    m_idToResource[visualID] = handleID;

    m_meshResourceTable[handleID].activesSubMeshes[surfaceIdx] = visualID;

    mesh.getResource().onLoaded([this, visualID, surfaceIdx] {
        const auto& meshRes = m_meshResourceTable[m_idToResource[visualID]].mesh.getResource();
        m_visualInstances.get(visualID)->init(
            meshRes.getVertices(surfaceIdx),
            meshRes.getIndices(surfaceIdx));
    });

    return visualID;
}

void Backend::clearMesh(renderID meshID)
{
    if (!m_visualInstances.tryRemove(meshID))
        return;

    auto& subMeshes = m_meshResourceTable[m_idToResource[meshID]].activesSubMeshes;
    if (subMeshes.size() <= 1) {
        // The resource is no longer used
        m_meshResourceTable.erase(m_idToResource[meshID]);
        m_idToResource.erase(meshID);
    } else {
        // We just clear one surface instance
        int surfaceToErase = -1;
        for (auto subMesh : subMeshes) {
            if (subMesh.second == meshID) {
                surfaceToErase = subMesh.first;
            }
        }
        if (surfaceToErase == -1) {
            GLX_CORE_ERROR("Could not clear subMesh visual instance: Mesh surface not found");
        } else {

            subMeshes.erase(surfaceToErase);
        }
    }
}

renderID Backend::instanciateTexture(ResourceHandle<Image> image)
{
    size_t handleID    = reinterpret_cast<size_t>(&image.getResource());
    auto it            = m_imageResourceTable.find(handleID);
    bool resourceExist = it != m_imageResourceTable.end();
    if (resourceExist) {
        return it->second.textureID;
    }

    renderID textureID = m_textureInstances.createResourceInstance();
    ImageHandle handle;
    handle.textureID               = textureID;
    handle.image                   = image;
    m_imageResourceTable[handleID] = handle;
    m_idToImageResource[textureID] = handleID;

    image.getResource().onLoaded([this, textureID] {
        size_t resourceIdx = m_idToImageResource[textureID];
        const auto& imgRes = m_imageResourceTable[resourceIdx].image.getResource();
        m_textureInstances.get(textureID)->init(imgRes.getData(), imgRes.getWidth(), imgRes.getHeight(), imgRes.getNbChannels());
    });

    return textureID;
}

void Backend::clearTexture(renderID textureID)
{
    if (!m_textureInstances.tryRemove(textureID))
        return;

    size_t imgID = m_idToImageResource[textureID];
    m_imageResourceTable.erase(imgID);
    m_idToImageResource.erase(textureID);
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