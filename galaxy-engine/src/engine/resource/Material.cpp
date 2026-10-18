#include "Material.hpp"

namespace Galaxy {

BaseMaterial::BaseMaterial(BaseMaterial&& other) noexcept
    : m_useTransparency(other.m_useTransparency)
    , m_transparency(other.m_transparency)
    , m_materialHandle(std::exchange(other.m_materialHandle, {}))
{
}

bool BaseMaterial::save(bool)
{
    return false;
}

bool BaseMaterial::load(YAML::Node&)
{
    return false;
}

void PBRMaterial::setImage(TextureType type, ResourceHandle<Image> image)
{
    m_useImage[type] = true;
    m_images[type] = image;
}

void TextureMaterial::setTexture(ResourceHandle<Image> image)
{
    m_texture = image;
    m_hasTexture = true;
}

void TextureMaterial::setTexture(TextureHandle texture)
{
    m_texture = texture;
    m_hasTexture = true;
}

void TextureMaterial::setTexture(CubemapHandle cubemap)
{
    m_texture = cubemap;
    m_hasTexture = true;
}

} // namespace Galaxy
