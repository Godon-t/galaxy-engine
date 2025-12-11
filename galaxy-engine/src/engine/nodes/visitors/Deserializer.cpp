#include "Deserializer.hpp"

#include <fstream>
#include <yaml-cpp/yaml.h>

#include "Log.hpp"
#include "common/YamlTranslation.hpp"
#include "nodes/Node.hpp"
#include "nodes/Node3D.hpp"
#include "nodes/NodeHelper.hpp"
#include "nodes/rendering/Camera.hpp"
#include "nodes/rendering/CornellBox.hpp"
#include "nodes/rendering/EnvironmentNode.hpp"
#include "nodes/rendering/MeshInstance.hpp"
#include "nodes/rendering/MultiMeshInstance.hpp"
#include "nodes/rendering/Sprite3D.hpp"
#include "nodes/rendering/lighting/GINode.hpp"
#include "nodes/rendering/lighting/PointLight.hpp"
#include "nodes/rendering/lighting/SpotLight.hpp"
#include "project/Project.hpp"
#include "types/Math.hpp"

using namespace math;

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

        targetScene.setUuid(m_data["Id"].as<uint64_t>());

        auto nodeHierarchy = m_data["Root"];
        if (nodeHierarchy) {
            m_currentYAMLNode               = nodeHierarchy;
            std::string nodeType            = nodeHierarchy["Type"].as<std::string>();
            std::shared_ptr<Node> sceneRoot = std::move(constructNode(nodeType));
            sceneRoot->accept(*this);

            targetScene.setNodePtr(sceneRoot);
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
void SceneDeSerializer::visit(CornellBox& node)
{
    visit(static_cast<Node3D&>(node));
}
void SceneDeSerializer::visit(MeshInstance& node)
{
    visit(static_cast<Node3D&>(node));

    if (m_currentYAMLNode["MeshResourceID"] && m_currentYAMLNode["SurfaceIndex"]) {
        uuid pathId(m_currentYAMLNode["MeshResourceID"].as<uint64_t>());
        std::string meshPath = Project::getPath(ProjectPathTypes::RESOURCE, pathId);
        int surfaceIdx       = m_currentYAMLNode["SurfaceIndex"].as<int>();

        if (surfaceIdx == -1)
            return;

        auto meshResource = ResourceManager::getInstance().load<Mesh>(meshPath);
        node.loadMesh(meshResource, surfaceIdx);
    }
}
void SceneDeSerializer::visit(MultiMeshInstance& node)
{
    visit(static_cast<Node3D&>(node));
}
void SceneDeSerializer::visit(Sprite3D& node)
{
    visit(static_cast<Node3D&>(node));

    if (m_currentYAMLNode["ImageID"]) {
        uuid imageID          = m_currentYAMLNode["ImageID"].as<uint64_t>();
        std::string imagePath = Project::getPath(ProjectPathTypes::RESOURCE, imageID);
        node.loadTexture(imagePath);
    }
}
void SceneDeSerializer::visit(EnvironmentNode& node)
{
    visit(static_cast<Node&>(node));
    if (m_currentYAMLNode["EnvID"]) {
        uuid envID          = m_currentYAMLNode["EnvID"].as<uint64_t>();
        std::string envPath = Project::getPath(ProjectPathTypes::RESOURCE, envID);
        node.loadEnv(ResourceManager::getInstance().load<Environment>(envPath));
    }
}

void SceneDeSerializer::deserializeLightCommon(Light& node)
{
    visit(static_cast<Node3D&>(node));
    if (m_currentYAMLNode["Intensity"]) {
        node.setIntensity(m_currentYAMLNode["Intensity"].as<float>());
    }
    if (m_currentYAMLNode["Color"]) {
        node.setColor(m_currentYAMLNode["Color"].as<vec3>());
    }
    if (m_currentYAMLNode["Range"]) {
        node.setRange(m_currentYAMLNode["Range"].as<float>());
    }
}

void SceneDeSerializer::visit(SpotLight& node)
{
    deserializeLightCommon(static_cast<Light&>(node));

    // if (m_currentYAMLNode["CutoffAngle"]) {
    //     node.setCutoffAngle(m_currentYAMLNode["CutoffAngle"].as<float>());
    // }
    // if (m_currentYAMLNode["OuterCutoffAngle"]) {
    //     node.setOuterCutoffAngle(m_currentYAMLNode["OuterCutoffAngle"].as<float>());
    // }
    // if (m_currentYAMLNode["CastShadows"]) {
    //     node.setCastShadows(m_currentYAMLNode["CastShadows"].as<bool>());
    // }
}

void SceneDeSerializer::visit(PointLight& node)
{
    deserializeLightCommon(static_cast<Light&>(node));
}

void SceneDeSerializer::visit(GINode& node)
{
    visit(static_cast<Node3D&>(node));
}

} // namespace Galaxy
