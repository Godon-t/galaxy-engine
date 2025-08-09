#include "pch.hpp"

#include "Root.hpp"

#include "visitors/Serializer.hpp"

Root::Root(ActionManager& actionManager, std::unique_ptr<Node> node)
{
    m_rootNode = std::move(node);
    actionManager.addListener([&](InputAction inputAction) { m_rootNode->handleInput(inputAction); });

    Galaxy::NodeSerializer serializer;
    serializer.serialize(*m_rootNode.get());
}

void Root::process()
{
    m_rootNode->updateTransformAndChilds();
}
