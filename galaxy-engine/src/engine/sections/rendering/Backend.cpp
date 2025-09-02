#include "Backend.hpp"

#include "Helper.hpp"
#include "OpenglHelper.hpp"
#include "engine/core/Log.hpp"
#include "gl_headers.hpp"

namespace Galaxy {
Backend::Backend(size_t maxSize)
{
    m_visuInstances.resize(maxSize);

    for (size_t i = 0; i < maxSize; i++) {
        m_freeIds.push(i);
    }

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

    if (m_freeIds.size() == 0)
        return -1;

    VisualInstance meshInstance;
    meshInstance.init(vertices, indices);

    renderID meshID                = m_freeIds.top();
    size_t listIdx                 = instanceCount;
    m_visuInstances[listIdx]       = std::make_pair(std::move(meshInstance), 1);
    m_visuIdxToInstanceId[listIdx] = meshID;
    m_instanceIdToVisuIdx[meshID]  = listIdx;

    m_freeIds.pop();
    instanceCount++;
    return meshID;
}

renderID Backend::instanciateMesh(ResourceHandle<Mesh> mesh)
{
    size_t handleID = reinterpret_cast<size_t>(&mesh.getResource());
    auto it         = m_resourceTable.find(handleID);
    if (it != m_resourceTable.end()) {
        m_visuInstances[it->second].second++;
        return it->second;
    }

    if (m_freeIds.size() == 0) {
        GLX_CORE_ERROR("No more free renderIDs");
        return -1;
    }

    renderID meshID = m_freeIds.top();
    size_t listIdx  = instanceCount;

    m_visuInstances[listIdx]       = std::make_pair(VisualInstance(), 1);
    m_visuIdxToInstanceId[listIdx] = meshID;
    m_instanceIdToVisuIdx[meshID]  = listIdx;

    m_freeIds.pop();
    instanceCount++;

    m_resourceTable[handleID] = meshID;
    m_idToResource[meshID]    = mesh;

    mesh.getResource().onLoaded([this, listIdx, meshID] {
        const auto& meshRes = m_idToResource[meshID].getResource();
        auto& meshInstance  = m_visuInstances[listIdx].first;
        meshInstance.init(
            meshRes.getVertices(),
            meshRes.getIndices());
    });

    return meshID;
}

void Backend::clearMesh(renderID meshID)
{
    m_visuInstances[meshID].second--;
    if (m_visuInstances[meshID].second > 0)
        return;

    instanceCount--;
    if (instanceCount == 0)
        return;

    size_t idxToDelete = m_instanceIdToVisuIdx[meshID];

    renderID movedMeshID               = m_visuIdxToInstanceId[instanceCount];
    m_instanceIdToVisuIdx[movedMeshID] = idxToDelete;
    m_visuIdxToInstanceId[idxToDelete] = movedMeshID;

    m_visuInstances[idxToDelete] = std::move(m_visuInstances[instanceCount]);

    m_freeIds.emplace(meshID);

    m_resourceTable.erase(reinterpret_cast<size_t>(&m_idToResource[meshID].getResource()));
    m_idToResource.erase(meshID);
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
        m_visuInstances[m_instanceIdToVisuIdx[command.draw.instanceId]].first.draw();
        break;
    }
    default:
        GLX_CORE_ERROR("Unknown render command");
        break;
    }
}

} // namespace Galaxy