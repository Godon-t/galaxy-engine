#include "pch.hpp"

#include "MeshInstance.hpp"

#include "project/Project.hpp"
#include "rendering/GPUInstances/VisualInstance.hpp"
#include "rendering/Renderer.hpp"
#include "resource/ResourceManager.hpp"

namespace Galaxy {
uint32_t MAX_IDs = 4096;
std::queue<uint32_t> initAvailableIDs(){
    std::queue<uint32_t> res;
    for(uint32_t id=1; id<MAX_IDs; id++){
        res.push(id);
    }
    return res;
}

std::queue<uint32_t> MeshInstance::s_availableIDs = initAvailableIDs();

MeshInstance::MeshInstance(std::string name) : Node3D(name)
    , m_meshSurfaceIdx(-1)
{
    m_renderItem.objectId = s_availableIDs.front();
    s_availableIDs.pop();
}

MeshInstance::~MeshInstance()
{
    if (m_renderItem.geometry)
        Renderer::getInstance().getBackend().clearMesh(m_renderItem.geometry);
    if (m_renderItem.material.has_value()) {
        Renderer::getInstance().getBackend().clearMaterial(m_renderItem.material.value());
    }

    s_availableIDs.push(m_renderItem.objectId);
    m_renderItem.objectId = 0;
}

void MeshInstance::draw()
{
    if (!m_renderItem.geometry)
        return;

    m_renderItem.transform = getTransform();
    Renderer::getInstance().getFrontend().addObjectToScene(m_renderItem);    
}

void MeshInstance::lightPassDraw()
{
    // if (m_geometry)
    //     Renderer::getInstance().getFrontend().submit(m_geometry, getTransform());
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
    if (m_renderItem.geometry) {
        Renderer::getInstance().getBackend().clearMesh(m_renderItem.geometry);
        m_renderItem.geometry = {};
    }
    if (m_renderItem.material.has_value()) {
        Renderer::getInstance().getBackend().clearMaterial(m_renderItem.material.value());
        m_renderItem.material = {};
        m_materialResource = {};
    }

    mesh.getResource().onLoaded([this, mesh, surfaceIdx] {
        m_renderItem.geometry = Renderer::getInstance().getBackend().instantiateMesh(mesh, surfaceIdx);

        if (!mesh.getResource().hasMaterial(surfaceIdx))
            return;

        setMaterial(mesh.getResource().getMaterial(surfaceIdx));

        m_renderItem.bounds = Renderer::getInstance().getBackend().getMeshBoundingVolume(m_renderItem.geometry);
    });

    m_meshResource   = mesh;
    m_meshSurfaceIdx = surfaceIdx;
}

void MeshInstance::setMaterial(ResourceHandle<BaseMaterial> material)
{
    if (m_renderItem.material.has_value())
        Renderer::getInstance().getBackend().clearMaterial(m_renderItem.material.value());
    m_renderItem.material = {};
    m_materialResource = material;
    material.getResource().onLoaded([this, material] {
        m_renderItem.material = Renderer::getInstance().getBackend().instantiateMaterial(material);
    });
}
} // namespace Galaxy
