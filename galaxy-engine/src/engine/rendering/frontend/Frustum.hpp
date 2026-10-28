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

        explicit Frustum(const RenderView& view){
            const mat4 rows = glm::transpose(view.projectionMatrix * view.viewMatrix);
            auto makePlane = [&view](const vec4& coefficients) {
                const float normalLength = length(vec3(coefficients));
                const vec3 normal = vec3(coefficients) / normalLength;
                const float distance = glm::dot(normal, view.position) + coefficients.w / normalLength;
                return Plane { view.position - normal * distance, normal };
            };

            leftFace = makePlane(rows[3] + rows[0]);
            rightFace = makePlane(rows[3] - rows[0]);
            bottomFace = makePlane(rows[3] + rows[1]);
            topFace = makePlane(rows[3] - rows[1]);
            nearFace = makePlane(rows[3] + rows[2]);
            farFace = makePlane(rows[3] - rows[2]);
        }
        
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
