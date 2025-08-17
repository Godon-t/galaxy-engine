#pragma once

#include "NodeVisitor.hpp"
#include "engine/project/Scene.hpp"
#include "pch.hpp"

#include <yaml-cpp/yaml.h>

namespace Galaxy {
class SceneSerializer : public NodeVisitor {
private:
    YAML::Emitter m_yaml;

public:
    void serialize(Scene& root, const char* outputPath);

    void visit(Node& node) override;
    void visit(Node3D& node) override;
    void visit(Camera& node) override;
    void visit(MeshInstance& node) override;
};
}