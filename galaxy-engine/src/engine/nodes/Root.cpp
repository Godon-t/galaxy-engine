#include "pch.hpp"

#include "Root.hpp"

#include "visitors/Serializer.hpp"

namespace Galaxy {
Root::Root(ActionManager& actionManager, std::unique_ptr<Node> node)
{
    m_rootNode = std::move(node);
    actionManager.addListener([&](EventAction evtAction) { m_rootNode->handleInput(evtAction); });

    Galaxy::NodeSerializer serializer;
    serializer.serialize(*m_rootNode.get());
}

void Root::process()
{
    m_rootNode->updateTransformAndChilds();
}
}
