#include "pch.hpp"

#include "Root.hpp"

namespace Galaxy {
void Root::process(double delta)
{
    if (!m_rootNode)
        return;
    m_rootNode->update(delta);
    m_rootNode->updateTransformAndChilds();
}

void Root::setRoot(std::shared_ptr<Node> node)
{
    m_rootNode = node;
    node->enterRoot();
}

void Root::handleEvent(Event& event)
{
    if (m_rootNode)
        m_rootNode->handleInput(event);
}
}
