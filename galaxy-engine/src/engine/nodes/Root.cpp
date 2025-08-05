#include "pch.hpp"

#include "Root.hpp"

#include "engine/input/InputManager.hpp"

Root::Root(InputManager &inputManager, std::unique_ptr<Node> node)
{
    rootNode = std::move(node);
    inputManager.addListener([&](InputAction inputAction) {rootNode->handleInput(inputAction);});
}

void Root::process()
{
    rootNode->updateTransformAndChilds();
}
