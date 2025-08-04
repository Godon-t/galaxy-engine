#pragma once

#include "engine/input/Action.hpp"

#include "engine/types/Math.hpp"

using namespace math;

class Node {
protected:
    Node* parent_ = nullptr;
    std::vector<std::unique_ptr<Node>> children_;  // Changement ici pour unique_ptr

public:
    Node() = default;
    Node(const Node&) = delete;  // Interdit la copie
    Node(Node&&) = default;
    
    virtual ~Node() = default;

    void setParent(Node* parent);

    Node* getParent() const;

    void addChild(std::unique_ptr<Node> child);
    void removeChild(Node* component);
    
    bool isLeaf() const {
        return children_.empty();
    }

    void destroy();

    virtual void updateTransformAndChilds(const mat4& matrix = mat4(1));
    virtual void forceUpdateTransformAndChilds(const mat4& matrix);

    void handleInput(const InputAction& inputAction);
    virtual void handleInputFromTop(const InputAction& inputAction){};
    virtual void handleInputFromBot(const InputAction& inputAction){};
};