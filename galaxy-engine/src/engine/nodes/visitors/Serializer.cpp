#include "Serializer.hpp"

#include "Log.hpp"

#include "common/YamlTranslation.hpp"
#include "nodes/Node.hpp"
#include "nodes/Node3D.hpp"
#include "nodes/rendering/Camera.hpp"
#include "nodes/rendering/EnvironmentNode.hpp"
#include "nodes/rendering/MeshInstance.hpp"
#include "nodes/rendering/MultiMeshInstance.hpp"
#include "nodes/rendering/Sprite3D.hpp"
#include "types/Math.hpp"

#include <fstream>
#include <iostream>

using namespace math;

#include <yaml-cpp/yaml.h>

namespace Galaxy {
void SceneSerializer::serialize(Scene& scene, const char* outputPath)
{
    m_yaml << YAML::BeginMap;
    m_yaml << YAML::Key << "Scene" << YAML::Value << "Name";
    m_yaml << YAML::Key << "Id" << YAML::Value << scene.getUuid();

    if (scene.getNodePtr().get() != nullptr) {
        m_yaml << YAML::Key << "Root" << YAML::BeginMap;
        scene.getNodePtr()->accept(*this);
        m_yaml << YAML::EndMap;
    }

    m_yaml << YAML::EndMap;

    std::ofstream fout(outputPath);
    fout << m_yaml.c_str();
}

void SceneSerializer::visit(Node& node)
{
    m_yaml << YAML::Key << "Type" << YAML::Value << node.getName().c_str();

    m_yaml << YAML::Key << "Childs" << YAML::BeginSeq;
    for (auto child : node.getChildren()) {
        m_yaml << YAML::BeginMap;
        child->accept(*this);
        m_yaml << YAML::EndMap;
    }
    m_yaml << YAML::EndSeq;
}

void SceneSerializer::visit(Node3D& node)
{
    visit(static_cast<Node&>(node));
    m_yaml << YAML::Key << "Transform" << YAML::Value << YAML::BeginMap;
    m_yaml << YAML::Key << "Position" << YAML::Value << node.getTransform()->getLocalPosition();
    m_yaml << YAML::Key << "Rotation" << YAML::Value << node.getTransform()->getLocalRotation();
    m_yaml << YAML::Key << "Scale" << YAML::Value << node.getTransform()->getLocalScale();
    m_yaml << YAML::EndMap;
}

void SceneSerializer::visit(Camera& node)
{
    visit(static_cast<Node3D&>(node));
}

void SceneSerializer::visit(MeshInstance& node)
{
    visit(static_cast<Node3D&>(node));
    m_yaml << YAML::Key << "MeshResourceID" << YAML::Value << node.getMeshResource().getResource().getResourceID();
    m_yaml << YAML::Key << "SurfaceIndex" << YAML::Value << node.getSurfaceIdx();
}
void SceneSerializer::visit(MultiMeshInstance& node)
{
    visit(static_cast<Node3D&>(node));
}
void SceneSerializer::visit(Sprite3D& node)
{
    visit(static_cast<Node3D&>(node));
    if (node.getImageResourceID() != 0)
        m_yaml << YAML::Key << "ImageID" << YAML::Value << node.getImageResourceID();
}
void SceneSerializer::visit(EnvironmentNode& node)
{
    visit(static_cast<Node&>(node));
    if (node.getEnvResourceID() != 0)
        m_yaml << YAML::Key << "EnvID" << YAML::Value << node.getEnvResourceID();
}
}
