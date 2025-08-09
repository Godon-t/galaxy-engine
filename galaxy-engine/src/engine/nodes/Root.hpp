#pragma once

#include "Node.hpp"
#include "engine/input/ActionManager.hpp"

class Root {
private:
    std::unique_ptr<Node> m_rootNode;

public:
    Root(ActionManager& actionManager, std::unique_ptr<Node> node);
    void process();
};
