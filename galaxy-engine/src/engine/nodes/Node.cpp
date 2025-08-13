#include "pch.hpp"

#include "Node.hpp"

namespace Galaxy {

void Node::setParent(Node* parent)
{
    this->m_parent = parent;
}

Node* Node::getParent() const
{
    return this->m_parent;
}

void Node::addChild(std::shared_ptr<Node> child)
{
    child->setParent(this);
    m_children.push_back(child);
    if (m_inRoot) {
        m_children[m_children.size() - 1]->enterRoot();
    }
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

std::vector<Node*> Node::getChildren() const
{
    // TODO: replace with an iterator
    std::vector<Node*> result;
    result.reserve(m_children.size());
    for (const auto& child : m_children) {
        result.push_back(child.get());
    }
    return result;
}

size_t Node::getChildCount() const
{
    return m_children.size();
}

void Node::destroy()
{
    m_children.clear();

    if (m_parent != nullptr) {
        m_parent->removeChild(this);
        m_parent = nullptr;
    }
}

void Node::enterRoot()
{
    enteringRoot();
    for (auto&& child : m_children) {
        child->enterRoot();
    }
    enteredRoot();
    m_inRoot = true;
}

void Node::update(double delta)
{
    process(delta);
    for (auto&& child : m_children) {
        child->update(delta);
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

void Node::handleInput(const Event& event)
{
    handleInputFromTop(event);
    for (auto&& child : m_children) {
        child->handleInput(event);
    }
    handleInputFromBot(event);
}

void Node::accept(NodeVisitor& visitor)
{
    visitor.visit(*this);
}
}
