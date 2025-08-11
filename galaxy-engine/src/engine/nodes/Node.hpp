#pragma once

#include "engine/event/ActionEvent.hpp"

#include "engine/types/Math.hpp"
#include "visitors/NodeVisitor.hpp"

using namespace math;

class Root;

namespace Galaxy {
class Node {
private:
    friend class NodeVisitor;
    friend class Root;
    void update(double delta);

protected:
    Node* m_parent = nullptr;
    std::vector<std::unique_ptr<Node>> m_children; // Changement ici pour unique_ptr

public:
    Node()            = default;
    Node(const Node&) = delete; // Interdit la copie
    Node(Node&&)      = default;

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

    virtual void process(double delta) { }

    virtual void updateTransformAndChilds(const mat4& matrix = mat4(1));
    virtual void forceUpdateTransformAndChilds(const mat4& matrix);

    void handleInput(const ActionEvent& eventAction);
    virtual void handleInputFromTop(const ActionEvent& eventAction) { }
    virtual void handleInputFromBot(const ActionEvent& eventAction) { }

    virtual void accept(NodeVisitor& visitor);
};
}