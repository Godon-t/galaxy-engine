#pragma once

#include "Node.hpp"
#include "engine/input/InputManager.hpp"

class Root {
private:
    std::unique_ptr<Node> m_rootNode;

public:
    Root(InputManager& inputManager, std::unique_ptr<Node> node);
    void process();
};
