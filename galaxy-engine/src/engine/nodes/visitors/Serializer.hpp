#pragma once

#include "NodeVisitor.hpp"
#include <yaml-cpp/yaml.h>
#include <typeindex>

namespace Galaxy {
    class NodeSerializer: public NodeVisitor{
    private:
        YAML::Emitter yaml;
    public:
        static std::unordered_map<std::type_index, char*> nodeNames;
        
        void serialize(Node& root);
        
        void visit(Node& node) override;
        void visit(Node3D& node) override;
        void visit(Camera& node) override;
        void visit(MeshInstance& node) override;

        void visitChildren(Node& node) override;
    private:
        char* getName(Node& node);
    };
}