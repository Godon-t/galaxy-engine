#include "MultiMeshInstance.hpp"

#include "MeshInstance.hpp"

#include "engine/project/Project.hpp"
#include "engine/resource/ResourceManager.hpp"

namespace Galaxy {
void MultiMeshInstance::enteredRoot()
{
}

MultiMeshInstance::~MultiMeshInstance()
{
}
void MultiMeshInstance::process(double delta)
{
}
void MultiMeshInstance::draw()
{
    Node3D::draw();
}
void MultiMeshInstance::accept(Galaxy::NodeVisitor& visitor)
{
    visitor.visit(*this);
}
void MultiMeshInstance::loadMesh(std::string path)
{
    auto meshRes = ResourceManager::getInstance().load<Mesh>(path);

    meshRes.getResource().onLoaded([this, path] {
        clearChilds();

        auto meshRes = ResourceManager::getInstance().load<Mesh>(path);
        int subCount = meshRes.getResource().getSubMeshesCount();
        for (int i = 0; i < subCount; i++) {
            std::shared_ptr<MeshInstance> meshInstance = std::make_shared<MeshInstance>();
            meshInstance->loadMesh(path, i);
            addChild(meshInstance);
        }
    });
}
} // namespace Galaxy
