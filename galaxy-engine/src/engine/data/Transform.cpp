#include "pch.hpp"

#include "Transform.hpp"

Transform::Transform()
    : m_globalModelMatrix(1.f)
    , m_pos(0.0f)
    , m_scale(1.0f)
    , m_rotationQuat(1.0f, 0.f, 0.f, 0.f)
    , m_rotationOrder(XYZ)
    , m_rotationEuler(0, 0, 0)
    , dirty(true)
{
}

Transform::Transform(Transform&& other) noexcept
    : m_pos(std::move(other.m_pos))
    , m_rotationQuat(std::move(other.m_rotationQuat))
    , m_scale(std::move(other.m_scale))
    , m_globalModelMatrix(std::move(other.m_globalModelMatrix))
    , dirty(other.dirty)
    , m_rotationOrder(other.m_rotationOrder)
    , m_rotationEuler(other.m_rotationEuler)
{
}

void Transform::computeModelMatrix(const mat4& parentGlobalModelMatrix)
{
    auto localMat       = getLocalModelMatrix();
    m_globalModelMatrix = parentGlobalModelMatrix * localMat;
    dirty               = false;
}

void Transform::computeModelMatrix()
{
    m_globalModelMatrix = getLocalModelMatrix();
    dirty               = false;
}

mat4 Transform::getLocalModelMatrix() const
{
    mat4 R = toMat4(m_rotationQuat);
    return math::translate(mat4(1.0f), m_pos)
        * R
        * math::scale(mat4(1.0f), m_scale);
}

mat4 Transform::getGlobalModelMatrix() const
{
    return m_globalModelMatrix;
}

void Transform::setLocalPosition(const vec3& position)
{
    m_pos = position;
    dirty = true;
}

vec3 Transform::getLocalPosition() const
{
    return m_pos;
}

vec3 Transform::getGlobalPosition() const
{
    return vec3(m_globalModelMatrix[3]);
}

void Transform::setLocalScale(const vec3& s)
{
    m_scale = s;
    dirty   = true;
}

vec3 Transform::getLocalScale() const
{
    return m_scale;
}

vec3 Transform::getGlobalScale(vec3 value) const
{
    vec3 res;
    res.x = length(vec3(m_globalModelMatrix[0])); // Basis vector X
    res.y = length(vec3(m_globalModelMatrix[1])); // Basis vector Y
    res.z = length(vec3(m_globalModelMatrix[2])); // Basis vector Z

    return res;
}

void Transform::setLocalRotation(const vec3& rotationAngles)
{
    m_rotationEuler       = rotationAngles;
    const mat4 transformX = math::rotate(mat4(1.0f),
        rotationAngles.x,
        vec3(1.0f, 0.0f, 0.0f));
    const mat4 transformY = math::rotate(mat4(1.0f),
        rotationAngles.y,
        vec3(0.0f, 1.0f, 0.0f));
    const mat4 transformZ = math::rotate(mat4(1.0f),
        rotationAngles.z,
        vec3(0.0f, 0.0f, 1.0f));

    mat4 rotationMatrix;
    switch (m_rotationOrder) {
    case YXZ:
        rotationMatrix = transformY * transformX * transformZ;
        break;
    case XYZ:
        rotationMatrix = transformX * transformY * transformZ;
        break;
    case ZYX:
        rotationMatrix = transformZ * transformY * transformX;
        break;
    }

    m_rotationQuat = toQuat(rotationMatrix);
    dirty          = true;
}

void Transform::setLocalRotation(const quat& q)
{
    m_rotationQuat  = q;
    m_rotationEuler = eulerAngles(m_rotationQuat);
    dirty           = true;
}

vec3 Transform::getLocalRotation() const
{
    return m_rotationEuler;
}

quat Transform::getLocalRotationQuat() const
{
    return m_rotationQuat;
}

void Transform::rotate(vec3 rotations)
{
    switch (m_rotationOrder) {
    case YXZ:
        localRotateY(rotations.y);
        localRotateX(rotations.x);
        localRotateZ(rotations.z);
        break;
    case XYZ:
        localRotateX(rotations.x);
        localRotateY(rotations.y);
        localRotateZ(rotations.z);
        break;
    case ZYX:
        localRotateZ(rotations.z);
        localRotateY(rotations.y);
        localRotateX(rotations.x);
        break;
    }
}

void Transform::rotate(float angle, vec3 axis)
{
    quat rot        = angleAxis(angle, axis);
    m_rotationQuat  = rot * m_rotationQuat;
    m_rotationEuler = eulerAngles(m_rotationQuat);
    dirty           = true;
}

void Transform::globalRotate(vec3 r)
{
    globalRotateX(r.x);
    globalRotateY(r.y);
    globalRotateZ(r.z);
}

void Transform::globalRotateX(float angle)
{
    rotate(angle, vec3(1, 0, 0));
}

void Transform::globalRotateY(float angle)
{
    rotate(angle, vec3(0, 1, 0));
}

void Transform::globalRotateZ(float angle)
{
    rotate(angle, vec3(0, 0, 1));
}

void Transform::localRotateX(float angle)
{
    vec3 axis = m_rotationQuat * vec3(1, 0, 0);
    rotate(angle, axis);
}

void Transform::localRotateY(float angle)
{
    vec3 axis = m_rotationQuat * vec3(0, 1, 0);
    rotate(angle, axis);
}

void Transform::localRotateZ(float angle)
{
    vec3 axis = m_rotationQuat * vec3(0, 0, 1);
    rotate(angle, axis);
}

// void Transform::scale(vec3 s)
// {
//     scale += s;
//     dirty
// }

void Transform::translate(vec3 translation)
{
    m_pos += translation;
    dirty = true;
}
