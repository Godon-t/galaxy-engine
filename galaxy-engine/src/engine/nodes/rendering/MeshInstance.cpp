#include "pch.hpp"

#include "MeshInstance.hpp"

#include "project/Project.hpp"
#include "rendering/GPUInstances/VisualInstance.hpp"
#include "rendering/renderer/Renderer.hpp"
#include "resource/ResourceManager.hpp"

namespace Galaxy {
void MeshInstance::generateTriangle()
{
    std::vector<Vertex> triangleVertices;
    Vertex v1, v2, v3;
    v1.position = vec3(-1, -1, 0);
    v1.texCoord = vec2(0, 0);
    v2.position = vec3(1, -1, 0);
    v2.texCoord = vec2(1, 0);
    v3.position = vec3(1, 1, 0);
    v3.texCoord = vec2(0, 1);

    triangleVertices.push_back(v1);
    triangleVertices.push_back(v2);
    triangleVertices.push_back(v3);

    std::vector<short unsigned int> triangleIndices;
    triangleIndices.push_back(0);
    triangleIndices.push_back(1);
    triangleIndices.push_back(2);

    m_renderId = Renderer::getInstance().instanciateMesh(triangleVertices, triangleIndices);
}

MeshInstance::~MeshInstance()
{
    if (m_renderIDInitialized) {
        Renderer::getInstance().clearMesh(m_renderId);
    }
}

void MeshInstance::enteredRoot()
{
}

void MeshInstance::process(double delta)
{
}

void MeshInstance::draw()
{
    Node3D::draw();
    if (m_renderIDInitialized) {
        Renderer::getInstance().bindTexture(m_textureID, "testTexture");
        Renderer::getInstance().submit(m_renderId, *getTransform());
    }
}

void MeshInstance::accept(Galaxy::NodeVisitor& visitor)
{
    visitor.visit(*this);
}

void MeshInstance::loadMesh(std::string path, int surfaceIdx)
{
    auto res              = ResourceManager::getInstance().load<Mesh>(path);
    m_renderId            = Renderer::getInstance().instanciateMesh(res, surfaceIdx);
    m_renderIDInitialized = true;

    m_meshResourceID = res.getResource().getResourceID();
    m_meshSurfaceIdx = surfaceIdx;

    auto texRes = ResourceManager::getInstance().load<Image>("Cube_BaseColor.png");
    m_textureID = Renderer::getInstance().instanciateTexture(texRes);
}
} // namespace Galaxy
