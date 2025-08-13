#pragma once

#include "NodeVisitor.hpp"
#include "engine/data/Transform.hpp"
#include "engine/nodes/Scene.hpp"
#include "pch.hpp"

#include <yaml-cpp/yaml.h>

namespace Galaxy {
class SceneDeSerializer : public NodeVisitor {
private:
    YAML::Node m_data;
    YAML::Node& m_currentYAMLNode;

    Transform deserializeTransform(YAML::Node& node);

    std::unique_ptr<Node> constructNode(std::string& type);

public:
    SceneDeSerializer();

    bool deserialize(Scene& targetScene, const char* path);

    void visit(Node& node) override;
    void visit(Node3D& node) override;
    void visit(Camera& node) override;
    void visit(MeshInstance& node) override;
};
}