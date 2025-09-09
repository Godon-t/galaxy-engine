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
        Renderer::getInstance().changeUsedProgram(BaseProgramEnum::PBR);
        Renderer::getInstance().submit(m_renderId, *getTransform());
    }
}

void MeshInstance::accept(NodeVisitor& visitor)
{
    visitor.visit(*this);
}

void MeshInstance::loadMesh(ResourceHandle<Mesh> mesh, int surfaceIdx)
{
    if (m_renderId != 0) {
        Renderer::getInstance().clearMesh(m_renderId);
    }

    mesh.getResource().onLoaded([this, mesh, surfaceIdx] {
        m_initialized = true;
        m_renderId    = Renderer::getInstance().instanciateMesh(mesh, surfaceIdx);
    });

    m_meshResource   = mesh;
    m_meshSurfaceIdx = surfaceIdx;
}
} // namespace Galaxy
