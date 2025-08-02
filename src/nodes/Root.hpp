#pragma once

#include <memory>

#include <src/nodes/Node.hpp>
#include <src/input/InputManager.hpp>

class Root{
private:
    std::unique_ptr<Node> rootNode;
public:
    Root(InputManager& inputManager, std::unique_ptr<Node> node);
    void process();
};
