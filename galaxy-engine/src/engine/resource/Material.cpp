#include "Material.hpp"

#include "ResourceManager.hpp"
#include "common/YamlTranslation.hpp"
#include "project/Project.hpp"

namespace Galaxy {
Material::Material()
{
    m_useImage.fill(false);
}

bool Material::save(bool)
{
    // Material persistence is temporarily disabled.
    return false;
}

bool Material::load(YAML::Node&)
{
    return false;
}

void Material::setImage(TextureType type, ResourceHandle<Image> image)
{
    m_useImage[type] = true;
    m_images[type]   = image;
}

} // namespace Galaxy
