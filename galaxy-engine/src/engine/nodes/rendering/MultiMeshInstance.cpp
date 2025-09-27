#include "MultiMeshInstance.hpp"

#include "MeshInstance.hpp"

#include "engine/project/Project.hpp"
#include "engine/resource/ResourceManager.hpp"

namespace Galaxy {
void MultiMeshInstance::draw()
{
    Node3D::draw();
}
void MultiMeshInstance::accept(Galaxy::NodeVisitor& visitor)
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

void MultiMeshInstance::loadMesh(std::string path)
{
    auto meshRes = ResourceManager::getInstance().load<Mesh>(path);

    meshRes.getResource().onLoaded([this, path] {
        clearChilds();

        auto meshRes = ResourceManager::getInstance().load<Mesh>(path);
        int subCount = meshRes.getResource().getSubMeshesCount();

        SubMeshTree& tree                         = meshRes.getResource().getRootTree();
        std::shared_ptr<Node3D> rootTransformNode = std::make_shared<Node3D>();
        addChild(rootTransformNode);
        instantiateFromTree(tree, meshRes, rootTransformNode);
    });
}
} // namespace Galaxy
