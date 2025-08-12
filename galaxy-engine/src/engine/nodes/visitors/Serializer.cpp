#include "Serializer.hpp"

#include "Log.hpp"

#include "engine/nodes/Node.hpp"
#include "engine/nodes/Node3D.hpp"
#include "engine/nodes/rendering/Camera.hpp"
#include "engine/nodes/rendering/MeshInstance.hpp"

#include <fstream>
#include <iostream>

namespace Galaxy {

void NodeSerializer::serialize(Node& root, const char* outputPath)
{
    m_yaml << YAML::BeginMap;
    m_yaml << YAML::Key << "NodeScene" << YAML::Value << "Name";

    root.accept(*this);

    m_yaml << YAML::EndMap;

    std::ofstream fout(outputPath);
    fout << m_yaml.c_str();
}

void NodeSerializer::visit(Node& node)
{
    m_yaml << YAML::Key << "Type" << YAML::Value << node.getName().c_str();
    visitChildren(node);
}

void NodeSerializer::visit(Node3D& node)
{
    visit(static_cast<Node&>(node));
    m_yaml << YAML::Key << "Transform" << YAML::Value << "(rotation, translation, scale)";
}

void NodeSerializer::visit(Camera& node)
{
    visit(static_cast<Node3D&>(node));
}

void NodeSerializer::visit(MeshInstance& node)
{
    visit(static_cast<Node3D&>(node));
}

void NodeSerializer::visitChildren(Node& node)
{
    m_yaml << YAML::Key << "Childs" << YAML::BeginSeq;
    for (auto child : node.getChildren()) {
        m_yaml << YAML::BeginMap;
        child->accept(*this);
        m_yaml << YAML::EndMap;
    }
    m_yaml << YAML::EndSeq;
}
}
