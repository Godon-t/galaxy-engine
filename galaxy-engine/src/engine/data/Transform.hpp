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
    quat m_rotationQuat      = quat();

    RotationOrderEnum m_rotationOrder = YXZ;

public:
    bool dirty = true;

    void computeModelMatrix(const mat4& parentGlobalModelMatrix);
    void computeModelMatrix();

    mat4 getLocalModelMatrix() const;
    mat4 getGlobalModelMatrix() const;

    void setLocalPosition(const vec3& position);
    vec3 getLocalPosition() const;
    vec3 getGlobalPosition() const;

    void setLocalScale(const vec3& s);
    vec3 getLocalScale() const;
    vec3 getGlobalScale(vec3 value) const;

    void setLocalRotation(const vec3& rotationAngles);
    void setLocalRotation(const quat& q);
    vec3 getLocalRotation() const;

    // vec3 applyRotation(vec3 vector);

    void rotate(vec3 r);
    void translate(vec3 t);
    // void scale(vec3 s);

    Transform();

    Transform(Transform&& other) noexcept;

    Transform& operator=(const Transform& other) noexcept
    {
        m_pos               = other.m_pos;
        m_rotationQuat      = other.m_rotationQuat;
        m_scale             = other.m_scale;
        m_globalModelMatrix = other.m_globalModelMatrix;
        dirty               = other.dirty;
        m_rotationOrder     = other.m_rotationOrder;
        return *this;
    }
    Transform& operator=(Transform&& other) noexcept
    {
        if (this != &other) {
            m_pos               = std::move(other.m_pos);
            m_rotationQuat      = std::move(other.m_rotationQuat);
            m_scale             = std::move(other.m_scale);
            m_globalModelMatrix = std::move(other.m_globalModelMatrix);
            dirty               = other.dirty;
            m_rotationOrder     = other.m_rotationOrder;

            other.dirty = true;
        }
        return *this;
    }
};