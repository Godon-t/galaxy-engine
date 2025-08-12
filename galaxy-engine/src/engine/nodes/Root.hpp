#pragma once

#include "Node.hpp"

namespace Galaxy {
class Root {
private:
    std::unique_ptr<Node> m_rootNode;

public:
    Root() { }
    void process(double delta);
    void setRoot(std::unique_ptr<Node> node);
    void handleEvent(Event& event);
    Node* getNodePtr() { return m_rootNode.get(); }
};
}
