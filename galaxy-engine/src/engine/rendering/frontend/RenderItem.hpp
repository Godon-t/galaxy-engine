#pragma once

#include "common/geometry/Shapes.hpp"
#include "data/Transform.hpp"
#include "rendering/GPUInstances/GpuResourceHandle.hpp"
#include "common/BitMask.hpp"

#include <optional>

using namespace Galaxy::geometry;

namespace Galaxy {

// Frontend description of something that may be rendered. It is not a GPU
// resource.
struct RenderItem {
    GeometryHandle geometry;
    std::optional<MaterialHandle> material;
    BitMask visibilityLayer;
    Sphere bounds;
    Transform transform;
};

} // namespace Galaxy
