#include "Scene.hpp"

#include "visitors/Serializer.hpp"

namespace Galaxy {
bool Scene::save()
{
    NodeSerializer serializer;
    serializer.serialize(*m_rootNode.get(), m_scenePath.c_str());
    return true;
}

bool Scene::isValid()
{
    return m_scenePath != "" && m_rootNode.get() != nullptr;
}

bool Scene::load(std::string path)
{
    m_scenePath = path;
    GLX_CORE_ERROR("Not yet implemented!");
    return false;
}
bool Scene::load(std::shared_ptr<Node> node)
{
    m_rootNode = node;
    return true;
}
}