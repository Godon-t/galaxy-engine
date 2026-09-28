#pragma once

#include "types/Math.hpp"

#include "Shapes.hpp"

using namespace math;


namespace Galaxy::geometry::collision
{
    bool isOnOrForwardPlane(const Sphere& sphere, const Plane& plane, const vec3& offset, float scale);

    bool intersects(const AxisAlignedBoundingBox& aabb, const Sphere& sphere);
} // namespace Galaxy
