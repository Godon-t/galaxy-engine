#include <src/nodes/Node.hpp>
#include <src/nodes/Node3D.hpp>
#include "Node.hpp"

void Node::setParent(Node* parent) {
    this->parent_ = parent;
}

Node *Node::getParent() const
{
    return this->parent_;
}

void Node::addChild(std::unique_ptr<Node> child)
{
    children_.push_back(std::move(child));
}

void Node::removeChild(Node *component)
{
    for (auto it = children_.begin(); it != children_.end(); ++it) {
        if (it->get() == component) {  // Vérifie si l'enfant correspond
            it->get()->setParent(nullptr);  // Déconnecte le parent
            children_.erase(it);  // Enlève l'enfant du vecteur
            break;
        }
    }
}

void Node::destroy() {
    children_.clear(); 

    if (parent_ != nullptr) {
        parent_->removeChild(this);
        parent_ = nullptr;
    }
}

void Node::updateTransformAndChilds(const mat4& matrix)
{
    for(auto&& child : children_){
        child->updateTransformAndChilds(matrix);
    }
}

void Node::forceUpdateTransformAndChilds(const mat4 &matrix)
{
    for(auto&& child : children_){
        child->forceUpdateTransformAndChilds(matrix);
    }
}

void Node::handleInput(const InputAction &inputAction)
{
    handleInputFromTop(inputAction);
    for(auto&& child : children_){
        child->handleInput(inputAction);
    }
    handleInputFromBot(inputAction);
}
