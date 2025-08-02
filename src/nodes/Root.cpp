#include <src/nodes/Root.hpp>

#include <src/input/InputManager.hpp>

Root::Root(InputManager &inputManager, std::unique_ptr<Node> node)
{
    rootNode = std::move(node);
    inputManager.addListener([&](InputAction inputAction) {rootNode->handleInput(inputAction);});
}
