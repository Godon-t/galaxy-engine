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
    POST_PROCESSING_PROBE,
    POST_PROCESSING_SSGI,
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
    DEPTH24STENCIL8,
    NONE
};

enum class FramebufferTextureFormat {
    None = 0,

    RGBA8,
    // RED_INTEGER,
    DEPTH24STENCIL8,
    DEPTH24RGBA8,
    DEPTH

    // Defaults
    // Depth = DEPTH24STENCIL8
};

enum CullMode {
    FRONT_CULLING,
    BACK_CULLING,
    BOTH_CULLING
};