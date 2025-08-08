#include "pch.hpp"

#include "Root.hpp"

#include "engine/input/InputManager.hpp"

Root::Root(InputManager& inputManager, std::unique_ptr<Node> node)
{
    m_rootNode = std::move(node);
    inputManager.addListener([&](InputAction inputAction) { m_rootNode->handleInput(inputAction); });
}

void Root::process()
{
    m_rootNode->updateTransformAndChilds();
}
