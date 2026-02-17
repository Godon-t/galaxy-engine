#pragma once

namespace Galaxy {
class Node;
class Node3D;
class CameraNode;
class MeshInstance;
class Sprite3D;
class EnvironmentNode;
class Light;
class SpotLight;
class PointLight;
class GINode;

class NodeVisitor {
public:
    virtual void visit(Node& node) {};
    virtual void visit(Node3D& node) {};
    virtual void visit(CameraNode& node) {};
    virtual void visit(MeshInstance& node) {};
    virtual void visit(Sprite3D& node) {};
    virtual void visit(EnvironmentNode& node) {};
    virtual void visit(SpotLight& node) {};
    virtual void visit(PointLight& node) {};
    virtual void visit(GINode& node) {};
};
}
