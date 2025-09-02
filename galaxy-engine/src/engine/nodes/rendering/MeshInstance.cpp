#include "pch.hpp"

#include "MeshInstance.hpp"

#include "engine/project/Project.hpp"
#include "engine/resource/ResourceManager.hpp"
#include "engine/sections/rendering/Renderer.hpp"
#include "engine/sections/rendering/VisualInstance.hpp"

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
    Renderer::getInstance().clearMesh(m_renderId);
}

void MeshInstance::enteredRoot()
{
    auto meshRes = ResourceManager::getInstance().load<Mesh>(Project::getProjectRootPath() + std::string("Cube.gltf"));
    m_renderId   = Renderer::getInstance().instanciateMesh(meshRes);
    // generateTriangle();
}

void MeshInstance::process(double delta)
{
}

void MeshInstance::draw()
{
    Node3D::draw();
    Renderer::getInstance().submit(m_renderId, *getTransform());
}

void MeshInstance::accept(Galaxy::NodeVisitor& visitor)
{
    visitor.visit(*this);
}
} // namespace Galaxy
