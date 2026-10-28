#include "Camera.hpp"

#include <array>

namespace Galaxy {

bool Camera::isCubemap() const
{
    return targetFramebuffer && std::holds_alternative<CubemapFramebufferHandle>(*targetFramebuffer);
}

std::vector<RenderView> Camera::getViews() const
{
    const bool cubemap = isCubemap();
    RenderView renderView;
    renderView.position = position;
    renderView.viewportSize = cubemap ? vec2(viewportSize.x) : viewportSize;
    renderView.viewportPosition = viewportPosition;
    renderView.visibilityMask = visibilityMask;
    renderView.frustumCulling = frustumCulling;
    renderView.targetFramebuffer = targetFramebuffer;
    const float aspect = renderView.viewportSize.x / renderView.viewportSize.y;
    renderView.projectionMatrix = perspective(
        radians(cubemap ? 90.0f : fovDegrees), aspect, zNear, zFar);

    if (!cubemap) {
        renderView.viewMatrix = lookAt(position, position + forward, up);
        return { renderView };
    }

    static const std::array<vec3, 6> directions {
        vec3(1, 0, 0), vec3(-1, 0, 0), vec3(0, 1, 0),
        vec3(0, -1, 0), vec3(0, 0, 1), vec3(0, 0, -1)
    };
    static const std::array<vec3, 6> ups {
        vec3(0, -1, 0), vec3(0, -1, 0), vec3(0, 0, 1),
        vec3(0, 0, -1), vec3(0, -1, 0), vec3(0, -1, 0)
    };

    std::vector<RenderView> views;
    views.reserve(directions.size());
    for (std::size_t face = 0; face < directions.size(); ++face) {
        renderView.viewMatrix = lookAt(position, position + directions[face], ups[face]);
        renderView.targetLayer = static_cast<int>(face);
        views.push_back(renderView);
    }
    return views;
}

} // namespace Galaxy
