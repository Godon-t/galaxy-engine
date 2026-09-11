#pragma once

#include "common/geometry/Shapes.hpp"
#include "data/Transform.hpp"
#include "rendering/renderer/resources/GpuResourceHandle.hpp"

#include <optional>

namespace Galaxy {

// Frontend description of something that may be rendered. It is not a GPU
// resource and it is not a backend command. Culling/sorting turns RenderItems
// into the current draw commands; a later pipeline refactor can add a pipeline
// override without changing GPU resource ownership.
struct RenderItem {
    GeometryHandle geometry;
    std::optional<MaterialHandle> material;
    Sphere bounds;
    Transform transform;
};

} // namespace Galaxy
