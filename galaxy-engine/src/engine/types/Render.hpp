#pragma once

#include "Math.hpp"

#include <cstddef>

using camID    = size_t;
using renderID = size_t;

struct Vertex {
    math::vec3 position;
    math::vec2 texCoord;
};