#pragma once

#include "types/Math.hpp"

using namespace math;

namespace Galaxy
{
    struct Plane {
        vec3 normal = {0,1,0};
        vec3 position = {0,0,0};
        
        Plane(){};
        Plane(const vec3& pos, const vec3& n):
            position(pos), 
            normal(n)
        {}

        float getSignedDistanceToPlane(const vec3& point) const
        {
            return dot(normal, position - point);
        }
    };

    struct Sphere {
        float radius;
        vec3 center;

        bool isOnOrForwardPlane(const Plane& plane) const{
            return plane.getSignedDistanceToPlane(center) > -radius;
        }
    };
} // namespace Galaxy
