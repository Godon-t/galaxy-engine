#pragma once

#include "common/geometry/Shapes.hpp"
#include "data/Transform.hpp"
#include "rendering/renderer/resources/GpuResourceHandle.hpp"

#include <optional>

using namespace Galaxy::geometry;

namespace Galaxy {

// Frontend description of something that may be rendered. It is not a GPU
// resource.
struct RenderItem {
    GeometryHandle geometry;
    std::optional<MaterialHandle> material;
    Sphere bounds;
    Transform transform;
};

} // namespace Galaxy
