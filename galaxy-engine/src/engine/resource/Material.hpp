#pragma once

#include "Image.hpp"
#include "Resource.hpp"
#include "ResourceHandle.hpp"
#include "rendering/renderer/resources/GpuResourceHandle.hpp"
#include "types/Material.hpp"
#include "types/Math.hpp"

#include <array>
#include <utility>
#include <variant>

namespace Galaxy {

class BaseMaterial : public ResourceBase {
public:
    BaseMaterial() = default;
    BaseMaterial(BaseMaterial&& other) noexcept;

    virtual MaterialType getType() const = 0;

    float getTransparency() const { return m_transparency; }
    void setTransparency(float value)
    {
        m_transparency = value;
        setUseTransparency(m_useTransparency || value < 1.0f);
    }
    void setUseTransparency(bool state) { m_useTransparency = state; }
    bool isUsingTransparency() const { return m_useTransparency; }

    MaterialHandle getGpuMaterialHandle() const { return m_materialHandle; }
    void setGpuMaterialHandle(MaterialHandle handle) { m_materialHandle = handle; }
    void notifyGpuInstanceDestroyed(MaterialHandle expectedHandle)
    {
        if (m_materialHandle == expectedHandle)
            m_materialHandle = {};
    }

    // Material persistence is temporarily disabled for both concrete types.
    bool save(bool recursive = true) final;
    bool load(YAML::Node& data) final;

private:
    bool m_useTransparency = false;
    float m_transparency = 1.0f;
    MaterialHandle m_materialHandle;
};

class PBRMaterial final : public BaseMaterial {
public:
    PBRMaterial() = default;
    PBRMaterial(PBRMaterial&& other) noexcept = default;

    MaterialType getType() const override { return MaterialType::PBR; }

    bool canUseImage(TextureType type) const { return m_useImage[type]; }
    ResourceHandle<Image> getImage(TextureType type) const { return m_images[type]; }
    void setImage(TextureType type, ResourceHandle<Image> image);

    math::vec3 getAlbedo() const { return m_albedo; }
    void setAlbedo(math::vec3 value) { m_albedo = value; }
    float getMetallic() const { return m_metallic; }
    void setMetallic(float value) { m_metallic = value; }
    float getRoughness() const { return m_roughness; }
    void setRoughness(float value) { m_roughness = value; }
    float getAmbient() const { return m_ambient; }
    void setAmbient(float value) { m_ambient = value; }

private:
    math::vec3 m_albedo = { 1.0f, 0.7f, 0.77f };
    float m_metallic = 0.5f;
    float m_roughness = 0.5f;
    float m_ambient = 1.0f;
    std::array<bool, TextureType::COUNT> m_useImage {};
    std::array<ResourceHandle<Image>, TextureType::COUNT> m_images;
};

class TextureMaterial final : public BaseMaterial {
public:
    using TextureSource = std::variant<ResourceHandle<Image>, TextureHandle, CubemapHandle>;

    TextureMaterial() = default;
    TextureMaterial(TextureMaterial&& other) noexcept = default;

    MaterialType getType() const override { return MaterialType::Texture; }

    void setTexture(ResourceHandle<Image> image);
    void setTexture(TextureHandle texture);
    void setTexture(CubemapHandle cubemap);
    const TextureSource& getTexture() const { return m_texture; }
    bool hasTexture() const { return m_hasTexture; }

private:
    TextureSource m_texture;
    bool m_hasTexture = false;
};

} // namespace Galaxy
