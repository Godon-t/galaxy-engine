#pragma once

#include "NodeVisitor.hpp"
#include "engine/project/Scene.hpp"
#include "pch.hpp"

#include <yaml-cpp/yaml.h>

namespace Galaxy {
class SceneSerializer : public NodeVisitor {
public:
    void serialize(Scene& root, const char* outputPath);

    void visit(Node& node) override;
    void visit(Node3D& node) override;
    void visit(Camera& node) override;
    void visit(CornellBox& node) override;
    void visit(MeshInstance& node) override;
    void visit(MultiMeshInstance& node) override;
    void visit(Sprite3D& node) override;
    void visit(EnvironmentNode& node) override;
    void visit(SpotLight& node) override;
    void visit(PointLight& node) override;
    void visit(GINode& node) override;

private:
    YAML::Emitter m_yaml;

    void visitLightCommon(Light& node);
};
}