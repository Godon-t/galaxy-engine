#include "pch.hpp"

#include "Node3D.hpp"

void Node3D::updateTransformAndChilds(const mat4& matrix)
{
    mat4 mat = m_parent == nullptr ? mat4(1) : matrix;
    if (m_transform.dirty) {
        forceUpdateTransformAndChilds(mat);
        return;
    }

    for (auto&& child : m_children) {
        child->updateTransformAndChilds(mat);
    }
}

void Node3D::forceUpdateTransformAndChilds(const mat4& matrix)
{
    m_transform.computeModelMatrix(matrix);

    for (auto&& child : m_children) {
        child->forceUpdateTransformAndChilds(m_transform.getGlobalModelMatrix());
    }
}

void Node3D::translate(vec3 translation)
{
    m_transform.translate(translation);
}
void Node3D::rotate(vec3 rotation)
{
    m_transform.rotate(rotation);
}
void Node3D::setScale(vec3 scale)
{
    m_transform.setLocalScale(scale);
}

void Node3D::accept(Galaxy::NodeVisitor& visitor)
{
    visitor.visit(*this);
}
