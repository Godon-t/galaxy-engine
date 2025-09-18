#pragma once

#include "resource/Material.hpp"
#include "types/Math.hpp"

namespace Galaxy {
struct MaterialInstance {
    MaterialInstance()
    {
        useImage.fill(false);
        images.fill(0);
    }
    math::vec3 albedo  = { 1.f, 0.7f, 0.77f };
    float metallic     = 0.5f;
    float roughness    = 0.5f;
    float ambient      = 1.0f;
    float transparency = 1.0f;

    std::array<bool, TextureType::COUNT> useImage;
    std::array<renderID, TextureType::COUNT> images;
};
} // namespace Galaxy
