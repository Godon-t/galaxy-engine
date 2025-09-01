#include "Serializer.hpp"

#include "Log.hpp"

#include "engine/nodes/Node.hpp"
#include "engine/nodes/Node3D.hpp"
#include "engine/nodes/rendering/Camera.hpp"
#include "engine/nodes/rendering/MeshInstance.hpp"
#include "engine/types/Math.hpp"

#include <fstream>
#include <iostream>

using namespace math;

#include <yaml-cpp/yaml.h>

namespace YAML {
Emitter& operator<<(Emitter& out, const vec3& v)
{
    out << Flow;
    out << BeginSeq << v.x << v.y << v.z << EndSeq;
    return out;
}

Emitter& operator<<(Emitter& out, const vec4& v)
{
    out << Flow;
    out << BeginSeq << v.x << v.y << v.z << v.w << EndSeq;
    return out;
}
}

namespace Galaxy {
void SceneSerializer::serialize(Scene& scene, const char* outputPath)
{
    m_yaml << YAML::BeginMap;
    m_yaml << YAML::Key << "Scene" << YAML::Value << "Name";
    m_yaml << YAML::Key << "Id" << YAML::Value << scene.getUuid();

    m_yaml << YAML::Key << "Root" << YAML::BeginMap;
    scene.getNodePtr()->accept(*this);
    m_yaml << YAML::EndMap;

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
}
}
