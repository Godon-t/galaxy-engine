#include "pch.hpp"

#include "Node.hpp"

void Node::setParent(Node* parent)
{
    this->m_parent = parent;
}

Node* Node::getParent() const
{
    return this->m_parent;
}

void Node::addChild(std::unique_ptr<Node> child)
{
    m_children.push_back(std::move(child));
}

void Node::removeChild(Node* component)
{
    for (auto it = m_children.begin(); it != m_children.end(); ++it) {
        if (it->get() == component) { // Vérifie si l'enfant correspond
            it->get()->setParent(nullptr); // Déconnecte le parent
            m_children.erase(it); // Enlève l'enfant du vecteur
            break;
        }
    }
}

void Node::destroy()
{
    m_children.clear();

    if (m_parent != nullptr) {
        m_parent->removeChild(this);
        m_parent = nullptr;
    }
}

void Node::updateTransformAndChilds(const mat4& matrix)
{
    for (auto&& child : m_children) {
        child->updateTransformAndChilds(matrix);
    }
}

void Node::forceUpdateTransformAndChilds(const mat4& matrix)
{
    for (auto&& child : m_children) {
        child->forceUpdateTransformAndChilds(matrix);
    }
}

void Node::handleInput(const InputAction& inputAction)
{
    handleInputFromTop(inputAction);
    for (auto&& child : m_children) {
        child->handleInput(inputAction);
    }
    handleInputFromBot(inputAction);
}
