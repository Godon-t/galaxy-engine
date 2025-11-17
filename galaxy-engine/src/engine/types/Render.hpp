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
    FILTER_IRRADIANCE
};

struct Vertex {
    math::vec3 position;
    math::vec2 texCoord;
    math::vec3 normal;
};

enum class FramebufferTextureFormat {
    None = 0,

    RGBA8,
    // RED_INTEGER,
    DEPTH24STENCIL8,
    DEPTH24RGBA8

    // Defaults
    // Depth = DEPTH24STENCIL8
};