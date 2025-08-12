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
    std::string m_name;

protected:
    Node* m_parent = nullptr;
    std::vector<std::unique_ptr<Node>> m_children; // Changement ici pour unique_ptr

public:
    Node(std::string name = "Node")
        : m_name(name) {};
    Node(const Node&) = delete; // Interdit la copie
    Node(Node&&)      = default;

    virtual ~Node() = default;

    void setParent(Node* parent);

    Node* getParent() const;

    void addChild(std::unique_ptr<Node> child);
    void removeChild(Node* component);
    std::vector<Node*> getChildren() const;
    size_t getChildCount() const;

    std::string getName() { return m_name; }

    bool isLeaf() const
    {
        return m_children.empty();
    }

    void destroy();

    virtual void process(double delta) { }

    virtual void updateTransformAndChilds(const mat4& matrix = mat4(1));
    virtual void forceUpdateTransformAndChilds(const mat4& matrix);

    void handleInput(const Event& event);
    virtual void handleInputFromTop(const Event& event) { }
    virtual void handleInputFromBot(const Event& event) { }

    virtual void accept(NodeVisitor& visitor);
};
}