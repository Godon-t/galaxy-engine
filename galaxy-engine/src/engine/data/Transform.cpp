#include "pch.hpp"

#include "Transform.hpp"

Transform::Transform(): globalModelMatrix(1.f), 
                        pos(0.0f), 
                        scale(1.0f), 
                        rotationQuat(1.0f, 0.f, 0.f, 0.f), 
                        rotationOrder(XYZ), 
                        dirty(true)
{}

Transform::Transform(Transform &&other) noexcept : pos(std::move(other.pos)),
                                                   rotationQuat(std::move(other.rotationQuat)),
                                                   scale(std::move(other.scale)),
                                                   globalModelMatrix(std::move(other.globalModelMatrix)),
                                                   dirty(other.dirty),
                                                   rotationOrder(other.rotationOrder)
{}

void Transform::computeModelMatrix(const mat4 &parentGlobalModelMatrix)
{
    globalModelMatrix = parentGlobalModelMatrix * getLocalModelMatrix();
    dirty = false;
}

void Transform::computeModelMatrix()
{
    globalModelMatrix = getLocalModelMatrix();
    dirty = false;
}

mat4 Transform::getLocalModelMatrix()
{
    mat4 R = toMat4(rotationQuat);
    return math::translate(mat4(1.0f), pos)
         * R
         * math::scale(mat4(1.0f), scale);
}

mat4 Transform::getGlobalModelMatrix()
{
    return globalModelMatrix;
}

void Transform::setLocalPosition(vec3 position)
{
    pos = position;
    dirty = true;
}

vec3 Transform::getLocalPosition(){
    return pos;
}

vec3 Transform::getGlobalPosition()
{
    return vec3(globalModelMatrix[3]);
}

void Transform::setLocalScale(const vec3 &s)
{
    scale = s;
    dirty = true;
}

vec3 Transform::getLocalScale() const
{
    return scale;
}

vec3 Transform::getGlobalScale(vec3 value)
{
    vec3 res;
    res.x = length(vec3(globalModelMatrix[0])); // Basis vector X
    res.y = length(vec3(globalModelMatrix[1])); // Basis vector Y
    res.z = length(vec3(globalModelMatrix[2])); // Basis vector Z
    
    return res;
}

void Transform::setLocalRotation(vec3 rotationAngles)
{
    const mat4 transformX = math::rotate(mat4(1.0f),
            radians(rotationAngles.x),
            vec3(1.0f, 0.0f, 0.0f));
    const mat4 transformY = math::rotate(mat4(1.0f),
            radians(rotationAngles.y),
            vec3(0.0f, 1.0f, 0.0f));
    const mat4 transformZ = math::rotate(mat4(1.0f),
            radians(rotationAngles.z),
            vec3(0.0f, 0.0f, 1.0f));
    
    mat4 rotationMatrix;
    switch (rotationOrder)
    {
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

    rotationQuat = toQuat(rotationMatrix);
    dirty = true;
}

void Transform::setLocalRotation(const quat &q)
{
    rotationQuat = q;
    dirty = true;
}

vec3 Transform::getLocalRotation()
{
    return degrees(eulerAngles(rotationQuat));
}

void Transform::rotate(vec3 rotations){
    vec3 rad = radians(rotations);
    quat dq = quat(rad);  
    rotationQuat = normalize(dq * rotationQuat);
    dirty = true;
}

// void Transform::scale(vec3 s)
// {
//     scale += s;
//     dirty
// }

void Transform::translate(vec3 translation)
{
    pos += translation;
    dirty = true;
}
