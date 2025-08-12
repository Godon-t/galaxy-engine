#pragma once

#include "Node.hpp"

namespace Galaxy {
class Root {
private:
    std::shared_ptr<Node> m_rootNode;

public:
    Root() { }
    void process(double delta);
    void setRoot(std::shared_ptr<Node> node);
    void handleEvent(Event& event);
    std::shared_ptr<Node> getNodePtr() { return m_rootNode; }
};
}
