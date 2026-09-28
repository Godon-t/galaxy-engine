#pragma once

#include "types/Math.hpp"
#include "Log.hpp"

using namespace math;

namespace Galaxy::geometry
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
            return dot(point - position, normal);
        }
    };

    struct Sphere {
        float radius;
        vec3 center;
    };
    
    struct AxisAlignedBoundingBox {
        vec3 center;
        vec3 halfSize;

        float sqDistPoint(const vec3& p) const
        {
            vec3 min = center - halfSize;
            vec3 max = center + halfSize;

            float sqDist = 0.0f;
            for( int i = 0; i < 3; i++ ){
                float v = p[i];
                if( v < min[i] ) 
                    sqDist += (min[i] - v) * (min[i] - v);
                if( v > max[i] ) 
                    sqDist += (v - max[i]) * (v - max[i]);
            }
            return sqDist;
        }
    };
} // namespace Galaxy
