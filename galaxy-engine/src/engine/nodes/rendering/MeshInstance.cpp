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
        Renderer::getInstance().getBackend().clearMesh(m_renderId);
    if (m_materialId){
        Renderer::getInstance().getFrontend().removeMaterialID(m_materialId);
        Renderer::getInstance().getBackend().clearMaterial(m_materialId);
    }
}

void MeshInstance::draw()
{
    if (m_materialId && m_renderId)
        Renderer::getInstance().getFrontend().submitPBR(m_renderId, m_materialId, *getTransform());
    // TODO: integrate in sceneContext
    else if (m_renderId)
        Renderer::getInstance().getFrontend().submit(m_renderId, *getTransform());
}

void MeshInstance::lightPassDraw()
{
    if (m_renderId)
        Renderer::getInstance().getFrontend().submit(m_renderId, *getTransform());
}

void MeshInstance::accept(NodeVisitor& visitor)
{
    visitor.visit(*this);
}

void instantiateFromTree(SubMeshTree& tree, ResourceHandle<Mesh> meshResource, std::shared_ptr<Node3D> parentNode)
{
    parentNode->translate(tree.translation);
    parentNode->setRotation(tree.rotation);
    parentNode->setScale(tree.scale);

    for (int meshIdx : tree.subMeshes) {
        std::shared_ptr<MeshInstance> meshInstance = std::make_shared<MeshInstance>();
        meshInstance->loadMesh(meshResource, meshIdx);
        parentNode->addChild(meshInstance);
    }

    for (SubMeshTree& treeChild : tree.childs) {
        if (treeChild.childs.size() == 0 && treeChild.subMeshes.size() == 0)
            continue;
        std::shared_ptr<Node3D> transformNode = std::make_shared<Node3D>();
        parentNode->addChild(transformNode);
        instantiateFromTree(treeChild, meshResource, transformNode);
    }
}

void MeshInstance::loadMesh(std::string path)
{
    auto meshRes = ResourceManager::getInstance().load<Mesh>(path);

    meshRes.getResource().onLoaded([this, path] {
        auto meshRes = ResourceManager::getInstance().load<Mesh>(path);
        int subCount = meshRes.getResource().getSubMeshesCount();

        SubMeshTree& tree                         = meshRes.getResource().getRootTree();
        std::shared_ptr<Node3D> rootTransformNode = std::make_shared<Node3D>();
        getParent()->addChild(rootTransformNode);
        instantiateFromTree(tree, meshRes, rootTransformNode);
        destroy();
    });
}

void MeshInstance::loadMesh(ResourceHandle<Mesh> mesh, int surfaceIdx)
{
    if (m_renderId != 0) {
        Renderer::getInstance().getBackend().clearMesh(m_renderId);
    }

    mesh.getResource().onLoaded([this, mesh, surfaceIdx] {
        m_renderId = Renderer::getInstance().getBackend().instanciateMesh(mesh, surfaceIdx);

        ResourceHandle<Material> mat = mesh.getResource().getMaterial(surfaceIdx);
        m_materialResource           = mat;
        mat.getResource().onLoaded([this, mat] {
            m_materialId = Renderer::getInstance().getBackend().instanciateMaterial(mat);
        });
    });

    m_meshResource   = mesh;
    m_meshSurfaceIdx = surfaceIdx;
}
} // namespace Galaxy
