#include "Scene.hpp"

#include "engine/nodes/visitors/Serializer.hpp"

namespace Galaxy {
bool Scene::save()
{
    SceneSerializer serializer;
    serializer.serialize(*this, m_scenePath.c_str());
    return true;
}

bool Scene::saveAs(const char* path, uuid id)
{
    SceneSerializer serializer;
    m_scenePath = std::string(path);
    m_uuid      = id;
    serializer.serialize(*this, m_scenePath.c_str());
    return false;
}

bool Scene::isValid()
{
    return m_scenePath != "" && m_rootNode.get() != nullptr;
}

bool Scene::load(std::shared_ptr<Node> node)
{
    m_rootNode = node;
    return true;
}
}