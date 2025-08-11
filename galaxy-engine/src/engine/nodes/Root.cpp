#include "pch.hpp"

#include "Root.hpp"

#include "visitors/Serializer.hpp"

namespace Galaxy {
Root::Root(ActionManager& actionManager, std::unique_ptr<Node> node)
{
    m_rootNode = std::move(node);
    actionManager.addListener([&](ActionEvent evtAction) { m_rootNode->handleInput(evtAction); });

    Galaxy::NodeSerializer serializer;
    serializer.serialize(*m_rootNode.get());
}

void Root::process(double delta)
{
    m_rootNode->update(delta);
    m_rootNode->updateTransformAndChilds();
}
}
