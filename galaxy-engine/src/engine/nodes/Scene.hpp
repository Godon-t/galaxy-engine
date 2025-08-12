#pragma once

#include "Node.hpp"

#include <memory>

namespace Galaxy {
class Scene {
public:
    Scene(std::string path = "")
        : m_scenePath(path)
    {
    }
    bool load(std::string path);
    bool load(std::shared_ptr<Node> node);
    bool save();
    bool isValid();
    std::shared_ptr<Node> getNodePtr() { return m_rootNode; }

private:
    std::shared_ptr<Node> m_rootNode;
    std::string m_scenePath;
};
}