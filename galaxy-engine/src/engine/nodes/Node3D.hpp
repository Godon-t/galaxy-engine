#pragma once

#include "engine/data/Transform.hpp"

#include "Node.hpp"

namespace Galaxy {
class Node3D : public Node {
protected:
    Transform m_transform;

public:
    Transform* getTransform() { return &m_transform; }
    void updateTransformAndChilds(const mat4& matrix) override;

    void forceUpdateTransformAndChilds(const mat4& matrix) override;

    void translate(vec3 translation);
    void rotate(vec3 rotation);
    void setScale(vec3 scale);

    void accept(Galaxy::NodeVisitor& visitor) override;
};
}
