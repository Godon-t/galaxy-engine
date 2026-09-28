#include "Collision.hpp"


namespace Galaxy{
    bool geometry::collision::isOnOrForwardPlane(const Sphere& sphere, const Plane& plane, const vec3& offset, float scale){
        return plane.getSignedDistanceToPlane(sphere.center * scale + offset) > -sphere.radius * scale * 2.f;
    }

    bool geometry::collision::intersects(const AxisAlignedBoundingBox& aabb, const Sphere& sphere){
        float sqDist = aabb.sqDistPoint(sphere.center);
    
        return sqDist <= sphere.radius * sphere.radius;
    }
}

