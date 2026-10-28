#pragma once

#include "common/BitMask.hpp"
#include "rendering/Commands.hpp"
#include "types/Math.hpp"

#include <optional>
#include <vector>

using namespace math;

namespace Galaxy
{
    struct RenderView {
        math::mat4 viewMatrix { 1.0f };
        math::mat4 projectionMatrix { 1.0f };
        math::vec3 position { 0.0f };
        math::vec2 viewportSize { 256.0f };
        math::vec2 viewportPosition { 0.0f };
        BitMask visibilityMask;
        bool frustumCulling = true;
        std::optional<RenderTargetHandle> targetFramebuffer;
        int targetLayer = -1;
    };
    
    struct Camera {
        vec3 position{0,0,0};
        vec3 forward {0,0,1};
        vec3 up      {0,1,0};
        vec3 right   {1,0,0};

        float zNear = 0.1f;
        float zFar = 9999.0f;
        float fovDegrees = 45.0f;
        vec2 viewportSize{256, 256};
        vec2 viewportPosition{0, 0};
        BitMask visibilityMask;
        bool frustumCulling = true;

        std::optional<RenderTargetHandle> targetFramebuffer;

        bool isCubemap() const;
        std::vector<RenderView> getViews() const;
    };    
} // namespace Galaxy
