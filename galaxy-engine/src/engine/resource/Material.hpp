#pragma once

#include "Image.hpp"
#include "Log.hpp"
#include "Resource.hpp"
#include "ResourceHandle.hpp"
#include "types/Math.hpp"

#include <assimp/scene.h>

namespace Galaxy {
enum TextureType {
    ALBEDO,
    NORMAL,
    METALLIC,
    ROUGHNESS,
    AO,
    COUNT
};

class MaterialsExtractor;

class Material : public ResourceBase {
public:
    Material();

    bool save(bool recursive = true) override;
    bool load(YAML::Node& data) override;

    inline bool canUseImage(TextureType type) const { return m_useImage[type]; }
    ResourceHandle<Image> getImage(TextureType type) const { return m_images[type]; }
    inline renderID getRenderID() const { return m_materialRenderID; }

    void setImage(TextureType type, ResourceHandle<Image> image);

    math::vec3 getAlbedo() const { return m_albedo; }
    float getMetallic() const { return m_metallic; }
    float getRoughness() const { return m_roughness; }
    float getAmbient() const { return m_ambient; }
    float getTransparency() const { return m_transparency; }
    void setTransparency(float value) { 
        m_transparency = value; 
        setUseTransparency(m_useTransparency || m_transparency < 1.f);
    }
    void setUseTransparency(bool state){ m_useTransparency = state; }
    bool isUsingTransparency() const {return m_useTransparency;}

private:
    friend class ResourceImporter;
    friend class ResourceSerializer;

    math::vec3 m_albedo  = { 1.f, 0.7f, 0.77f };
    float m_metallic     = 0.5f;
    float m_roughness    = 0.5f;
    float m_ambient      = 1.0f;
    float m_transparency = 1.0f;

    bool m_useTransparency = false;

    renderID m_materialRenderID = 0;

    std::array<bool, TextureType::COUNT> m_useImage;
    std::array<ResourceHandle<Image>, TextureType::COUNT> m_images;
};
} // namespace Galaxy
