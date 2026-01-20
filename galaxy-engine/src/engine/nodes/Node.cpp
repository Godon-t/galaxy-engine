#include "pch.hpp"

#include "Node.hpp"

namespace Galaxy {

size_t Node::s_rootID = 0;
std::unordered_map<size_t, std::weak_ptr<Node>> Node::s_nodeByIds;

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
    s_nodeByIds[child->id] = child;
    if (m_inRoot) {
        m_children[m_children.size() - 1]->enterRoot();
    }
}

void Node::removeChild(Node* component)
{
    for (auto it = m_children.begin(); it != m_children.end(); ++it) {
        if (it->get() == component) {
            it->get()->setParent(nullptr);
            m_children.erase(it);
            break;
        }
    }
}

void Node::clearChilds()
{
    for (auto child : m_children) {
        child->destroy(true);
    }
    m_children.clear();
}

bool Node::nodeExists(size_t id)
{
    auto it = s_nodeByIds.find(id);
    return it != s_nodeByIds.end() && !it->second.expired();
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

void Node::destroy(bool destroyByParent)
{
    clearChilds();

    if (!destroyByParent && m_parent != nullptr) {
        m_parent->removeChild(this);
    }
    m_parent = nullptr;
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
    if (m_paused)
        return;
    for (auto&& child : m_children) {
        child->update(delta);
    }
}

void Node::draw()
{
    for (auto&& child : m_children) {
        child->draw();
    }
}

void Node::lightPassDraw()
{
    for (auto&& child : m_children) {
        child->lightPassDraw();
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
    if (m_paused)
        return;

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
