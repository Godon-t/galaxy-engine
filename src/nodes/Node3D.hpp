#include <src/data/Transform.hpp>

#include <src/nodes/Node.hpp>

class Node3D: Node {
public:
    Transform transform;

    void updateTransformAndChilds(const mat4 &matrix) override;

    void forceUpdateTransformAndChilds(const mat4 &matrix) override;

};
