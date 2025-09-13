#pragma once

#include "Math.hpp"

#include <cstddef>

using camID    = size_t;
using renderID = size_t;

enum ProgramType {
    NONE,
    PBR,
    SKYBOX
};

struct Vertex {
    math::vec3 position;
    math::vec2 texCoord;
    math::vec3 normal;
};