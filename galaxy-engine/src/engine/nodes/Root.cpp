#include "pch.hpp"

#include "Root.hpp"

namespace Galaxy {
void Root::process(double delta)
{
    m_rootNode->update(delta);
    m_rootNode->updateTransformAndChilds();
}

void Root::setRoot(std::shared_ptr<Node> node)
{
    m_rootNode = node;
}

void Root::handleEvent(Event& event)
{
    m_rootNode->handleInput(event);
}
}
