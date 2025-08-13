#include "Scene.hpp"

#include "visitors/Deserializer.hpp"
#include "visitors/Serializer.hpp"

namespace Galaxy {
bool Scene::save()
{
    SceneSerializer serializer;
    serializer.serialize(*this, m_scenePath.c_str());
    return true;
}

bool Scene::isValid()
{
    return m_scenePath != "" && m_rootNode.get() != nullptr;
}

bool Scene::load(std::string path)
{
    SceneDeSerializer deserializer;
    m_scenePath = path;
    return deserializer.deserialize(*this, path.c_str());
}
bool Scene::load(std::shared_ptr<Node> node)
{
    m_rootNode = node;
    return true;
}
}