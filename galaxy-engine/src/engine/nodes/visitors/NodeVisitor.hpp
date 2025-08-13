#pragma once

namespace Galaxy {
class Node;
class Node3D;
class Camera;
class MeshInstance;

class NodeVisitor {
public:
    virtual void visit(Node& node) {};
    virtual void visit(Node3D& node) {};
    virtual void visit(Camera& node) {};
    virtual void visit(MeshInstance& node) {};
};
}
