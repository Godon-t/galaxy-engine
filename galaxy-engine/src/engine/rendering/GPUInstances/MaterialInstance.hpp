#pragma once

#include "types/Material.hpp"
#include "rendering/renderer/resources/GpuResourceHandle.hpp"
#include "types/Math.hpp"

#include <array>
#include <variant>

namespace Galaxy {
struct MaterialInstance {
    MaterialInstance()
    {
        useImage.fill(false);
        images.fill(TextureHandle {});
    }
    MaterialType type = MaterialType::PBR;
    math::vec3 albedo  = { 1.f, 0.7f, 0.77f };
    float metallic     = 0.5f;
    float roughness    = 0.5f;
    float ambient      = 1.0f;
    float transparency = 1.0f;

    std::array<bool, TextureType::COUNT> useImage;
    std::array<TextureHandle, TextureType::COUNT> images;
    bool hasTexture = false;
    std::variant<TextureHandle, CubemapHandle> texture;
};
} // namespace Galaxy
