#include "Serializer.hpp"

#include "Log.hpp"

#include <iostream>
#include "engine/nodes/Node.hpp"
#include "engine/nodes/Node3D.hpp"
#include "engine/nodes/rendering/MeshInstance.hpp"
#include "engine/nodes/rendering/Camera.hpp"

std::unordered_map<std::type_index, char*> Galaxy::NodeSerializer::nodeNames = {
    {typeid(Node), "Node"},
    {typeid(Node3D), "Node3D"},
    {typeid(Camera), "Camera"},
    {typeid(MeshInstance), "MeshInstance"}
};

void Galaxy::NodeSerializer::serialize(Node &root)
{
    yaml << YAML::BeginMap;
    yaml << YAML::Key << "NodeScene" << YAML::Value << "Name";
    
    root.accept(*this);
    
    yaml << YAML::EndMap;
    yaml << "\n";
    
    std::cout << yaml.c_str();
}

void Galaxy::NodeSerializer::visit(Node &node)
{
    yaml << YAML::Key << "Type" << YAML::Value << getName(node);
    visitChildren(node);
}

void Galaxy::NodeSerializer::visit(Node3D &node)
{
    visit(static_cast<Node&>(node));
    yaml << YAML::Key << "Transform" << YAML::Value << "(rotation, translation, scale)";
}

void Galaxy::NodeSerializer::visit(Camera &node)
{
    visit(static_cast<Node3D&>(node));
}

void Galaxy::NodeSerializer::visit(MeshInstance &node)
{
    visit(static_cast<Node3D&>(node));
}

void Galaxy::NodeSerializer::visitChildren(Node &node)
{
    yaml << YAML::Key << "Childs" << YAML::BeginSeq;
    for(auto child : node.getChildren()){
        yaml << YAML::BeginMap;
        child->accept(*this);
        yaml << YAML::EndMap;
    }
    yaml << YAML::EndSeq;
}

char *Galaxy::NodeSerializer::getName(Node &node)
{
    auto it = nodeNames.find(typeid(node));
    if (it != nodeNames.end()) {
        return it->second;
    } else {
        return "Untilted";
    }
}
