#include "pch.hpp"

#include "Root.hpp"

#include "visitors/Serializer.hpp"

namespace Galaxy {
void Root::process(double delta)
{
    m_rootNode->update(delta);
    m_rootNode->updateTransformAndChilds();
}

void Root::setRoot(std::unique_ptr<Node> node)
{
    m_rootNode = std::move(node);
    Galaxy::NodeSerializer serializer;
    serializer.serialize(*m_rootNode.get());
}

void Root::handleEvent(Event& event)
{
    m_rootNode->handleInput(event);
}
}
