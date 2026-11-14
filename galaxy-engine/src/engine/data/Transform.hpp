#pragma once

#include "engine/types/Math.hpp"

using namespace math;

enum RotationOrderEnum {
    XYZ,
    YXZ,
    ZYX
};
struct Transform {
private:
    vec3 m_pos               = { 0.0f, 0.0f, 0.0f };
    vec3 m_scale             = { 1.0f, 1.0f, 1.0f };
    mat4 m_globalModelMatrix = mat4(1.f);
    mat4 m_parentModelMatrix = mat4(1.f);
    quat m_rotationQuat      = quat();

    vec3 m_rotationEuler = vec3(0, 0, 0);

    RotationOrderEnum m_rotationOrder = YXZ;

public:
    bool dirty = true;

    void computeModelMatrix(const mat4& parentGlobalModelMatrix);
    void computeModelMatrix();

    mat4 getLocalModelMatrix() const;
    mat4 getGlobalModelMatrix() const;
    void setGlobalModelMatrix(mat4 global);

    void setLocalPosition(const vec3& position);
    vec3 getLocalPosition() const;
    vec3 getGlobalPosition() const;

    void setLocalScale(const vec3& s);
    vec3 getLocalScale() const;
    vec3 getGlobalScale() const;

    void setLocalRotation(const vec3& rotationAngles);
    void setLocalRotation(const quat& q);
    vec3 getLocalRotation() const;
    quat getLocalRotationQuat() const;

    // vec3 applyRotation(vec3 vector);

    void rotate(vec3 r);
    void globalRotate(vec3 r);
    void rotate(float angle, vec3 axis);

    void globalRotateX(float angle);
    void globalRotateY(float angle);
    void globalRotateZ(float angle);

    void localRotateX(float angle);
    void localRotateY(float angle);
    void localRotateZ(float angle);

    void translate(vec3 t);
    // void scale(vec3 s);

    Transform();
};