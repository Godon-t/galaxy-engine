#pragma once

#include "Node.hpp"

class SceneSerializer{
public:
    SceneSerializer(Node& rootNode);

    void Serialize(std::string& filePath);
    void SerializeRuntime(std::string& filePath);
    
    void Deserialize(std::string& filePath);
    void DeserializeRuntile(std::string& filePath);
};
