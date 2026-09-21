#pragma once

#include "Math.hpp"

#include <cstddef>

using ShaderParameterValue = std::variant<
    bool,
    int,
    float,
    math::vec2,
    math::vec3,
    math::ivec3,
    math::vec4,
    math::mat4
>;

using camID    = size_t;
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

enum struct TextureWrap {
    CLAMP_TO_EDGE,
    CLAMP_TO_BORDER,
    REPEAT
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
