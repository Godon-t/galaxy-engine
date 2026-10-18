#pragma once

namespace Galaxy {

enum TextureType {
    ALBEDO,
    NORMAL,
    METALLIC,
    ROUGHNESS,
    AO,
    COUNT
};

enum class MaterialType {
    PBR,
    Texture
};

struct MaterialRenderInfo {
    MaterialType type = MaterialType::PBR;
    bool isTransparent = false;
    bool isCubemap = false;
};

} // namespace Galaxy
