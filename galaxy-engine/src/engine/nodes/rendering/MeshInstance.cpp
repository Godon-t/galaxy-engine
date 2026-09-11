#include "pch.hpp"

#include "MeshInstance.hpp"

#include "project/Project.hpp"
#include "rendering/GPUInstances/VisualInstance.hpp"
#include "rendering/renderer/Renderer.hpp"
#include "resource/ResourceManager.hpp"

namespace Galaxy {
MeshInstance::~MeshInstance()
{
    if (m_geometry)
        Renderer::getInstance().getBackend().clearMesh(m_geometry);
    if (m_materialHandle) {
        Renderer::getInstance().getFrontend().removeMaterialID(m_materialHandle);
        Renderer::getInstance().getBackend().clearMaterial(m_materialHandle);
    }
}

void MeshInstance::draw()
{
    if (!m_geometry)
        return;

    std::optional<MaterialHandle> material;
    if (m_materialHandle)
        material = m_materialHandle;
    Renderer::getInstance().addObjectToScene(m_geometry, material, *getTransform());
}

void MeshInstance::lightPassDraw()
{
    if (m_geometry)
        Renderer::getInstance().getFrontend().submit(m_geometry, *getTransform());
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
    // GLX-TODO: potential bug, we are not sure it is the only instance of geometry and material. Could potentialy invalidate GPU resource for other instances
    if (m_geometry) {
        Renderer::getInstance().getBackend().clearMesh(m_geometry);
        m_geometry = {};
    }
    if (m_materialHandle) {
        Renderer::getInstance().getBackend().clearMaterial(m_materialHandle);
        m_materialHandle = {};
        m_materialResource = {};
    }

    mesh.getResource().onLoaded([this, mesh, surfaceIdx] {
        m_geometry = Renderer::getInstance().getBackend().instanciateMesh(mesh, surfaceIdx);

        if (!mesh.getResource().hasMaterial(surfaceIdx))
            return;

        ResourceHandle<Material> mat = mesh.getResource().getMaterial(surfaceIdx);
        m_materialResource           = mat;
        mat.getResource().onLoaded([this, mat] {
            m_materialHandle = Renderer::getInstance().getBackend().instanciateMaterial(mat);
        });
    });

    m_meshResource   = mesh;
    m_meshSurfaceIdx = surfaceIdx;
}
} // namespace Galaxy
