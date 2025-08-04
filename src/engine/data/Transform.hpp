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
    vec3 pos = { 0.0f, 0.0f, 0.0f };
    vec3 scale = { 1.0f, 1.0f, 1.0f };
    mat4 globalModelMatrix = mat4(1.f);
    quat rotationQuat = quat();

    RotationOrderEnum rotationOrder = YXZ;
    
public:
    bool dirty = true;


    void computeModelMatrix(const mat4& parentGlobalModelMatrix);
    void computeModelMatrix();

    mat4 getLocalModelMatrix();
    mat4 getGlobalModelMatrix();
    
    
    void setLocalPosition(vec3 position);
    vec3 getLocalPosition();
    vec3 getGlobalPosition();

    void setLocalScale(const vec3 &s);
    vec3 getLocalScale() const;
    vec3 getGlobalScale(vec3 value);
    
    void setLocalRotation(vec3 rotationAngles);
    void setLocalRotation(const quat &q) ;
    vec3 getLocalRotation();

    // vec3 applyRotation(vec3 vector);

    void rotate(vec3 r);
    void translate(vec3 t);
    // void scale(vec3 s);

    Transform();

    Transform(Transform&& other) noexcept;

    Transform& operator=(Transform&& other) noexcept {
        if (this != &other) {
            pos = std::move(other.pos);
            rotationQuat = std::move(other.rotationQuat);
            scale = std::move(other.scale);
            globalModelMatrix = std::move(other.globalModelMatrix);
            dirty = other.dirty;
            rotationOrder = other.rotationOrder;

            other.dirty = true;
        }
        return *this;
    }
};