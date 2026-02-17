#pragma once

#include "types/Math.hpp"
#include "data/Transform.hpp"
#include "data/Camera.hpp"
#include "common/geometry/Shapes.hpp"

using namespace math;

namespace Galaxy
{
    struct Frustum {
        Plane topFace;
        Plane bottomFace;

        Plane rightFace;
        Plane leftFace;

        Plane farFace;
        Plane nearFace;

        Frustum(const vec3& position, const vec3& direction, const vec3& right, const vec3& up, const vec2& dimmensions, float zNear, float zFar){
            vec2 halfSize = zFar * dimmensions * 0.5f;
            const vec3 frontMultFar = zFar * direction;

            nearFace = {position + zNear * direction, direction};
            farFace = {position + frontMultFar, -direction};
            
            rightFace = {position, cross(frontMultFar - right * halfSize.x, up)};
            leftFace = {position, cross(up, frontMultFar + right * halfSize.x)};

            topFace = {position, cross(right, frontMultFar - up * halfSize.y)};
            topFace = {position, cross(frontMultFar + up * halfSize.y, right)};
        }

        Frustum(Camera* camera): Frustum(camera->position, camera->forward, camera->right, camera->up, camera->dimmensions, camera->zNear, camera->zFar){}
        
        static bool isSphereInFrustum(const Sphere& sphere, const Frustum& frustum, const Transform& transform) {
            const vec3 globalScale = transform.getGlobalScale();
            const vec3 globalCenter{ transform.getGlobalModelMatrix() * vec4(sphere.center, 1.0f)};
            const float maxScale = std::max(std::max(globalScale.x, globalScale.y), globalScale.z);

            return  sphere.isOnOrForwardPlane(frustum.leftFace) &&
                    sphere.isOnOrForwardPlane(frustum.rightFace) &&
                    sphere.isOnOrForwardPlane(frustum.nearFace) &&
                    sphere.isOnOrForwardPlane(frustum.farFace) &&
                    sphere.isOnOrForwardPlane(frustum.topFace) &&
                    sphere.isOnOrForwardPlane(frustum.bottomFace);
        }
    };
} // namespace Galaxy
