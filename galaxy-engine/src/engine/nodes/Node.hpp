#pragma once

#include "engine/event/Action.hpp"

#include "engine/types/Math.hpp"
#include "visitors/NodeVisitor.hpp"

using namespace math;

namespace Galaxy {
class Node {
private:
    friend class NodeVisitor;

protected:
    Node* m_parent = nullptr;
    std::vector<std::unique_ptr<Node>> m_children; // Changement ici pour unique_ptr

public:
    Node() = default;
    Node(const Node&) = delete; // Interdit la copie
    Node(Node&&) = default;

    virtual ~Node() = default;

    void setParent(Node* parent);

    Node* getParent() const;

    void addChild(std::unique_ptr<Node> child);
    void removeChild(Node* component);
    std::vector<Node*> getChildren() const;

    bool isLeaf() const
    {
        return m_children.empty();
    }

    void destroy();

    virtual void updateTransformAndChilds(const mat4& matrix = mat4(1));
    virtual void forceUpdateTransformAndChilds(const mat4& matrix);

    void handleInput(const EventAction& eventAction);
    virtual void handleInputFromTop(const EventAction& eventAction) {};
    virtual void handleInputFromBot(const EventAction& eventAction) {};

    virtual void accept(NodeVisitor& visitor);
};
}