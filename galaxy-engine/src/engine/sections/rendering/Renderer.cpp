#include "Renderer.hpp"
#include "pch.hpp"

#include "OpenglHelper.hpp"
#include "VisualInstance.hpp"
#include "engine/Helper.hpp"

#include "Core.hpp"

namespace Galaxy {
const size_t maxSize = 512;
Renderer::Renderer()
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
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // glEnable(GL_CULL_FACE);
    glDisable(GL_CULL_FACE);

    m_mainProgram = std::move(Program(galaxy::engineRes("shaders/vertex.glsl"), galaxy::engineRes("shaders/fragment.glsl")));

    checkOpenGLErrors("Renderer constructor");
}

Renderer::~Renderer() { }

Renderer& Renderer::getInstance()
{
    static Renderer renderer;
    return renderer;
}

void Renderer::beginSceneRender(mat4& camTransform)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_viewMatrix = CameraManager::processViewMatrix(camTransform);
    m_mainProgram.use();
    m_mainProgram.updateViewMatrix(m_viewMatrix);
}

void Renderer::submit(const Transform& transform, renderID meshID)
{
    auto modelMatrix = transform.getGlobalModelMatrix();
    m_mainProgram.updateModelMatrix(modelMatrix);
    m_visuInstances[m_instanceIdToVisuIdx[meshID]].draw();
}

void Renderer::endSceneRender()
{
}

void Renderer::renderFrame()
{
}

renderID Renderer::instanciateMesh(std::vector<Vertex>& vertices, std::vector<short unsigned int>& indices)
{
    if (m_freeIds.size() == 0)
        return -1;

    VisualInstance meshInstance;
    meshInstance.init(vertices, indices);

    renderID meshID                = m_freeIds.top();
    size_t listIdx                 = instanceCount;
    m_visuInstances[listIdx]       = std::move(meshInstance);
    m_visuIdxToInstanceId[listIdx] = meshID;
    m_instanceIdToVisuIdx[meshID]  = listIdx;

    m_freeIds.pop();
    instanceCount++;
    return meshID;
}

void Renderer::clearMesh(renderID meshID)
{
    size_t idxToDelete = m_instanceIdToVisuIdx[meshID];

    instanceCount--;

    renderID movedMeshID               = m_visuIdxToInstanceId[instanceCount];
    m_instanceIdToVisuIdx[movedMeshID] = idxToDelete;
    m_visuIdxToInstanceId[idxToDelete] = movedMeshID;

    m_visuInstances[idxToDelete] = std::move(m_visuInstances[instanceCount]);

    m_freeIds.emplace(meshID);
}
}
