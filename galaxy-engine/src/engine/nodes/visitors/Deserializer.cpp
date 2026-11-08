#include "Deserializer.hpp"

#include <fstream>
#include <yaml-cpp/yaml.h>

#include "Log.hpp"
#include "common/YamlTranslation.hpp"
#include "nodes/Node.hpp"
#include "nodes/Node3D.hpp"
#include "nodes/NodeHelper.hpp"
#include "nodes/rendering/CameraNode.hpp"
#include "nodes/rendering/EnvironmentNode.hpp"
#include "nodes/rendering/MeshInstance.hpp"
#include "nodes/rendering/Sprite3D.hpp"
#include "nodes/rendering/lighting/GINode.hpp"
#include "nodes/rendering/lighting/PointLight.hpp"
#include "nodes/rendering/lighting/ProbeNode.hpp"
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
void SceneDeSerializer::visit(CameraNode& node)
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
    // Older scenes stored the same source under SkyboxID.
    const uuid environmentMapID(m_currentYAMLNode["EnvironmentMapID"] ? m_currentYAMLNode["EnvironmentMapID"].as<uint64_t>()
        : m_currentYAMLNode["SkyboxID"] ? m_currentYAMLNode["SkyboxID"].as<uint64_t>() : 0);
    const auto light = m_currentYAMLNode["DirectionalLight"];
    const bool hasDirectionalLight = static_cast<bool>(light);
    const vec3 direction = light && light["Direction"] ? light["Direction"].as<vec3>() : node.getDirection();
    const vec3 color = light && light["Color"] ? light["Color"].as<vec3>() : node.getColor();
    const float intensity = light && light["Intensity"] ? light["Intensity"].as<float>() : node.getIntensity();
    visit(static_cast<Node&>(node));
    if (hasDirectionalLight) {
        node.setDirection(direction);
        node.setColor(color);
        node.setIntensity(intensity);
    }
    if (environmentMapID != 0) {
        node.loadEnvironmentMap(Project::getPath(ProjectPathTypes::RESOURCE, environmentMapID));
    }
}

void SceneDeSerializer::deserializeLightCommon(Light& node)
{
    visit(static_cast<Node3D&>(node));
    node.setIntensity(m_currentYAMLNode["Intensity"].as<float>());
    node.setColor(m_currentYAMLNode["Color"].as<vec3>());
    node.setRange(m_currentYAMLNode["Range"].as<float>());
    node.setCastShadows(m_currentYAMLNode["Shadows"].as<bool>());
}

void SceneDeSerializer::visit(SpotLight& node)
{
    deserializeLightCommon(static_cast<Light&>(node));
    node.setInnerCutoffAngle(m_currentYAMLNode["InnerCutoffAngle"].as<float>());
    node.setOuterCutoffAngle(m_currentYAMLNode["OuterCutoffAngle"].as<float>());
}

void SceneDeSerializer::visit(PointLight& node)
{
    deserializeLightCommon(static_cast<Light&>(node));
}

void SceneDeSerializer::visit(GINode& node)
{
    visit(static_cast<Node3D&>(node));

    if (m_currentYAMLNode["Grid"]) {
        node.setGrid(m_currentYAMLNode["Grid"].as<ivec3>());
    }
    if (m_currentYAMLNode["SpaceBetween"]) {
        node.setSpaceBetween(m_currentYAMLNode["SpaceBetween"].as<float>());
    }
    if (m_currentYAMLNode["ProbeResolution"]) {
        node.setProbeResolution(m_currentYAMLNode["ProbeResolution"].as<unsigned int>());
    }
}

void SceneDeSerializer::visit(ProbeNode& node)
{
    const auto probeData = m_currentYAMLNode;
    visit(static_cast<Node3D&>(node));
    if (probeData["CubemapID"]) {
        const uuid resourceID(probeData["CubemapID"].as<uint64_t>());
        node.loadCubemap(Project::getPath(ProjectPathTypes::RESOURCE, resourceID));
    }
}

} // namespace Galaxy
