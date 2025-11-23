#pragma once

namespace Galaxy {
class Node;
class Node3D;
class Camera;
class MeshInstance;
class MultiMeshInstance;
class Sprite3D;
class EnvironmentNode;
class SpotLight;

class NodeVisitor {
public:
    virtual void visit(Node& node) {};
    virtual void visit(Node3D& node) {};
    virtual void visit(Camera& node) {};
    virtual void visit(MeshInstance& node) {};
    virtual void visit(MultiMeshInstance& node) {};
    virtual void visit(Sprite3D& node) {};
    virtual void visit(EnvironmentNode& node) {};
    virtual void visit(SpotLight& node) {};
};
}
