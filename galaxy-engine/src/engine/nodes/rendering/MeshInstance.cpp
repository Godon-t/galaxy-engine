#include "pch.hpp"

#include "MeshInstance.hpp"

#include "project/Project.hpp"
#include "rendering/GPUInstances/VisualInstance.hpp"
#include "rendering/renderer/Renderer.hpp"
#include "resource/ResourceManager.hpp"

namespace Galaxy {
MeshInstance::~MeshInstance()
{
    if (m_initialized) {
        Renderer::getInstance().clearMesh(m_renderId);
    }
}

void MeshInstance::draw()
{
    if (m_initialized) {
        Renderer::getInstance().submit(m_renderId, *getTransform());
    }
}

void MeshInstance::accept(Galaxy::NodeVisitor& visitor)
{
    visitor.visit(*this);
}

void MeshInstance::loadMesh(std::string path, int surfaceIdx)
{
    auto res = ResourceManager::getInstance().load<Mesh>(path);
    res.getResource().onLoaded([this] {
        m_initialized = true;
    });
    m_renderId = Renderer::getInstance().instanciateMesh(res, surfaceIdx);

    m_meshResourceID = res.getResource().getResourceID();
    m_meshSurfaceIdx = surfaceIdx;
}
} // namespace Galaxy
