#pragma once

#include "engine/data/Transform.hpp"

#include "Node.hpp"

namespace Galaxy {
class Node3D : public Node {
protected:
    Transform m_transform;
    virtual void enteringRoot() override {};
    virtual void enteredRoot() override {};

public:
    Node3D(std::string name = "Node3D")
        : Node(name)
    {
    }
    Transform* getTransform() { return &m_transform; }
    const Transform* getTransform() const { return &m_transform; }
    void setTransform(Transform& transform);
    
    virtual void updateTransformAndChilds(const mat4& matrix) override;
    virtual void forceUpdateTransformAndChilds(const mat4& matrix) override;

    void translate(vec3 translation);
    void rotate(vec3 rotation);
    void setRotation(quat rotation);
    void setScale(vec3 scale);

    void accept(Galaxy::NodeVisitor& visitor) override;

    inline virtual void draw() override { Node::draw(); }
    inline virtual void lightPassDraw() override { Node::lightPassDraw(); }
};
}
