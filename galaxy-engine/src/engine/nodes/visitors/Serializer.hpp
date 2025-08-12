#pragma once

#include "NodeVisitor.hpp"
#include <typeindex>
#include <yaml-cpp/yaml.h>

namespace Galaxy {
class NodeSerializer : public NodeVisitor {
private:
    YAML::Emitter m_yaml;

public:
    static std::unordered_map<std::type_index, char*> nodeNames;

    void serialize(Node& root, const char* outputPath);

    void visit(Node& node) override;
    void visit(Node3D& node) override;
    void visit(Camera& node) override;
    void visit(MeshInstance& node) override;

    void visitChildren(Node& node) override;
};
}