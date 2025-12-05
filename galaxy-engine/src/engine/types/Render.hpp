#pragma once

#include "Math.hpp"

#include <cstddef>

using camID    = size_t;
using renderID = size_t;
using lightID  = size_t;

enum ProgramType {
    NONE,
    PBR,
    SKYBOX,
    TEXTURE,
    UNICOLOR,
    POST_PROCESSING,
    FILTER_IRRADIANCE,
    SHADOW_DEPTH,
    COMPUTE_OCTAHEDRAL
};

struct Vertex {
    math::vec3 position;
    math::vec2 texCoord;
    math::vec3 normal;
};

enum class TextureFormat {
    RED,
    RGB,
    RGBA,
    DEPTH,
    RADIAL_DISTANCE,
    DEPTH24STENCIL8
};

enum CullMode {
    FRONT_CULLING,
    BACK_CULLING,
    BOTH_CULLING
};