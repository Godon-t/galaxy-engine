#include "Deserializer.hpp"

#include <fstream>
#include <yaml-cpp/yaml.h>

#include "Log.hpp"
#include "engine/nodes/Node.hpp"
#include "engine/nodes/Node3D.hpp"
#include "engine/nodes/NodeHelper.hpp"
#include "engine/nodes/rendering/Camera.hpp"
#include "engine/nodes/rendering/MeshInstance.hpp"
#include "engine/types/Math.hpp"

using namespace math;

namespace YAML {
template <>
struct convert<vec3> {
    static Node encode(const vec3& vec)
    {
        Node node;
        node.push_back(vec.x);
        node.push_back(vec.y);
        node.push_back(vec.z);
        return node;
    }
    static bool decode(const Node& node, vec3& rhs)
    {
        if (!node.IsSequence() || node.size() != 3) {
            return false;
        }

        rhs.x = node[0].as<float>();
        rhs.y = node[1].as<float>();
        rhs.z = node[2].as<float>();
        return true;
    }
};
}

namespace Galaxy {
Galaxy::SceneDeSerializer::SceneDeSerializer()
    : m_currentYAMLNode(m_data)
{
}

bool SceneDeSerializer::deserialize(Scene& targetScene, const char* path)
{
    std::ifstream stream(path);
    std::stringstream strStream;
    strStream << stream.rdbuf();

    m_data = YAML::Load(strStream.str());
    if (m_data["Scene"]) {
        std::string sceneName = m_data["Scene"].as<std::string>();
        GLX_CORE_TRACE("Deserializing scene '{0}'", sceneName);

        std::string strUuid = m_data["Id"].as<std::string>();
        targetScene.setUuid(uuid(strUuid));

        auto nodeHierarchy = m_data["Root"];
        if (nodeHierarchy) {
            m_currentYAMLNode               = nodeHierarchy;
            std::string nodeType            = nodeHierarchy["Type"].as<std::string>();
            std::shared_ptr<Node> sceneRoot = std::move(constructNode(nodeType));
            sceneRoot->accept(*this);

            targetScene.load(sceneRoot);
        }

        return true;
    }

    GLX_CORE_ASSERT(false, "Not a scene to deserialize!");
    return false;
}

Transform SceneDeSerializer::deserializeTransform(YAML::Node& node)
{
    Transform res;
    auto test  = node["Position"];
    vec3 pos   = node["Position"].as<vec3>();
    vec3 rot   = node["Rotation"].as<vec3>();
    vec3 scale = node["Scale"].as<vec3>();
    res.setLocalPosition(pos);
    res.setLocalRotation(rot);
    res.setLocalScale(scale);
    return res;
}

void SceneDeSerializer::visit(Node& node)
{
    auto yamlNode = m_currentYAMLNode;
    for (auto child : yamlNode["Childs"]) {
        m_currentYAMLNode               = child.as<YAML::Node>();
        std::string type                = child["Type"].as<std::string>();
        std::unique_ptr<Node> childNode = constructNode(type);
        childNode->accept(*this);
        node.addChild(std::move(childNode));
    }
}
void SceneDeSerializer::visit(Node3D& node)
{
    auto yamlTransform = m_currentYAMLNode["Transform"];
    auto nodeTransform = deserializeTransform(yamlTransform);
    node.setTransform(nodeTransform);
    visit(static_cast<Node&>(node));
}
void SceneDeSerializer::visit(Camera& node)
{
    visit(static_cast<Node3D&>(node));
}
void SceneDeSerializer::visit(MeshInstance& node)
{
    visit(static_cast<Node3D&>(node));
}
} // namespace Galaxy
