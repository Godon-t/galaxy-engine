#pragma once

#include "Log.hpp"

#include "types/Math.hpp"
#include "data/Transform.hpp"
#include "data/Camera.hpp"

#include "common/geometry/Shapes.hpp"
#include "common/geometry/Collision.hpp"

using namespace math;
using namespace Galaxy::geometry;

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
            vec2 halfSize;
            halfSize.y = zFar * tanf(radians(45.f) * 0.5f);
            halfSize.x = halfSize.y * (dimmensions.x / dimmensions.y);
            const vec3 frontMultFar = zFar * direction;

            nearFace = {position + zNear * direction, direction};
            farFace = {position + frontMultFar, -direction};
            
            rightFace  = {position, normalize(cross(frontMultFar + right * halfSize.x, up))};
            leftFace   = {position, normalize(cross(up, frontMultFar - right * halfSize.x))};

            topFace    = {position, normalize(cross(right, frontMultFar + up * halfSize.y))};
            bottomFace = {position, normalize(cross(frontMultFar - up * halfSize.y, right))};
        }

        Frustum(Camera* camera): Frustum(camera->position, camera->forward, camera->right, camera->up, camera->dimmensions, camera->zNear, camera->zFar){}
        
        static bool isSphereInFrustum(const Sphere& sphere, const Frustum& frustum, const Transform& transform) {
            const vec3 globalScale = transform.getGlobalScale();
            const vec3 globalCenter{ transform.getGlobalModelMatrix() * vec4(sphere.center, 1.0f)};
            const float maxScale = std::max(std::max(globalScale.x, globalScale.y), globalScale.z);

            return  collision::isOnOrForwardPlane(sphere,frustum.leftFace, transform.getGlobalPosition(), maxScale) &&
                    collision::isOnOrForwardPlane(sphere,frustum.rightFace, transform.getGlobalPosition(), maxScale) &&
                    collision::isOnOrForwardPlane(sphere,frustum.nearFace, transform.getGlobalPosition(), maxScale) &&
                    collision::isOnOrForwardPlane(sphere,frustum.farFace, transform.getGlobalPosition(), maxScale) &&
                    collision::isOnOrForwardPlane(sphere,frustum.topFace, transform.getGlobalPosition(), maxScale) &&
                    collision::isOnOrForwardPlane(sphere,frustum.bottomFace, transform.getGlobalPosition(), maxScale);
        }
    };
} // namespace Galaxy
