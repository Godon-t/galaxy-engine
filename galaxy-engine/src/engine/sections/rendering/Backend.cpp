#include "Backend.hpp"

#include "Helper.hpp"
#include "OpenglHelper.hpp"
#include "engine/core/Log.hpp"
#include "gl_headers.hpp"

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

    m_mainProgram = std::move(Program(engineRes("shaders/vertex.glsl"), engineRes("shaders/fragment.glsl")));

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
    auto it            = m_resourceTable.find(handleID);
    bool resourceExist = it != m_resourceTable.end();
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
        meshHandle.mesh           = mesh;
        m_resourceTable[handleID] = meshHandle;
    }
    m_idToResource[visualID] = handleID;

    m_resourceTable[handleID].activesSubMeshes[surfaceIdx] = visualID;

    mesh.getResource().onLoaded([this, visualID, surfaceIdx] {
        const auto& meshRes = m_resourceTable[m_idToResource[visualID]].mesh.getResource();
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

    auto& subMeshes = m_resourceTable[m_idToResource[meshID]].activesSubMeshes;
    if (subMeshes.size() <= 1) {
        // The resource is no longer used
        m_resourceTable.erase(m_idToResource[meshID]);
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

void Backend::processCommands(std::vector<RenderCommand>& commands)
{
    m_mainProgram.use();
    for (auto& command : commands) {
        processCommand(command);
    }
}

void Backend::processCommand(RenderCommand& command)
{
    switch (command.type) {
    case RenderCommandType::clear: {
        auto& clearColor = command.clear.color;
        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        break;
    }
    case RenderCommandType::setView:
        m_mainProgram.updateViewMatrix(command.setView.view);
        break;
    case RenderCommandType::setProjection:
        m_mainProgram.updateProjectionMatrix(command.setProjection.projection);
        break;
    case RenderCommandType::draw: {
        auto& modelMatrix = command.draw.model;
        m_mainProgram.updateModelMatrix(modelMatrix);
        m_visualInstances.get(command.draw.instanceId)->draw();
        break;
    }
    default:
        GLX_CORE_ERROR("Unknown render command");
        break;
    }
}

} // namespace Galaxy