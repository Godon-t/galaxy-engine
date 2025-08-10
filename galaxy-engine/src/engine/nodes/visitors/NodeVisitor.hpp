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
    virtual void visitChildren(Node& node) {};
};
}

// class SceneSerializer{
// public:
//     SceneSerializer(Node& rootNode);

//     void Serialize(std::string& filePath);
//     void SerializeRuntime(std::string& filePath);

//     void Deserialize(std::string& filePath);
//     void DeserializeRuntile(std::string& filePath);
// };
