#include <src/data/Transform.hpp>

#include <src/nodes/Node.hpp>

class Node3D: public Node {
protected:
    Transform transform;
public:
    Transform* getTransform() {return &transform; }
    void updateTransformAndChilds(const mat4 &matrix) override;

    void forceUpdateTransformAndChilds(const mat4 &matrix) override;

};
