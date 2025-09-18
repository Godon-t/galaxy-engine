#include "pch.hpp"

#include "MeshInstance.hpp"

#include "project/Project.hpp"
#include "rendering/GPUInstances/VisualInstance.hpp"
#include "rendering/renderer/Renderer.hpp"
#include "resource/ResourceManager.hpp"

namespace Galaxy {
MeshInstance::~MeshInstance()
{
    if (m_renderId)
        Renderer::getInstance().clearMesh(m_renderId);
    if (m_materialId)
        Renderer::getInstance().clearMaterial(m_materialId);
}

void MeshInstance::draw()
{
    if (m_materialId && m_renderId)
        Renderer::getInstance().submitPBR(m_renderId, m_materialId, *getTransform());
    else if (m_renderId)
        Renderer::getInstance().submit(m_renderId, *getTransform());
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
        m_renderId = Renderer::getInstance().instanciateMesh(mesh, surfaceIdx);

        ResourceHandle<Material> mat = mesh.getResource().getMaterial(surfaceIdx);
        mat.getResource().onLoaded([this, mat] {
            m_materialId = Renderer::getInstance().instanciateMaterial(mat);
        });
    });

    m_meshResource   = mesh;
    m_meshSurfaceIdx = surfaceIdx;
}
} // namespace Galaxy
