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
    Material()
    {
        m_useImage.fill(false);
    }

    bool save() override
    {
        for (int i = 0; i < m_useImage.size(); i++) {
            if (m_useImage[i]) {
                m_images[i].getResource().save();
            }
        }
        return ResourceSerializer::serialize(*this);
    }
    bool load(YAML::Node& data) override
    {
        GLX_CORE_ASSERT(false, "No material loading implemented!");
    }

    inline bool canUseImage(TextureType type) const { return m_useImage[type]; }
    ResourceHandle<Image> getImage(TextureType type) const { return m_images[type]; }
    inline renderID getRenderID() const { return m_materialRenderID; }

    void setImage(TextureType type, ResourceHandle<Image> image)
    {
        m_useImage[type] = true;
        m_images[type]   = image;
    }

private:
    friend class ResourceImporter;

    math::vec3 m_albedo = { 1.f, 0.7f, 0.77f };
    float m_metallic    = 0.5f;
    float m_roughness   = 0.5f;
    float m_ao          = 1.0f;

    renderID m_materialRenderID;

    std::array<bool, TextureType::COUNT> m_useImage;
    std::array<ResourceHandle<Image>, TextureType::COUNT> m_images;
};
} // namespace Galaxy
