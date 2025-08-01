#include <src/nodes/Node3D.hpp>


void Node3D::updateTransformAndChilds(const mat4& matrix)
{
    mat4 mat = parent_ == nullptr ? mat4(1) : matrix;
    if (transform.dirty) {
        forceUpdateTransformAndChilds(mat);
        return;
    }

    for(auto&& child : children_){
        child->updateTransformAndChilds(mat);
    }
}

void Node3D::forceUpdateTransformAndChilds(const mat4 &matrix)
{
    transform.computeModelMatrix(matrix);
    
    for(auto&& child : children_){
        child->forceUpdateTransformAndChilds(transform.getGlobalModelMatrix());
    }
}
