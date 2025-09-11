#include "Material.hpp"

#include "ResourceManager.hpp"
#include "common/YamlTranslation.hpp"
#include "project/Project.hpp"

namespace Galaxy {
Material::Material()
{
    m_useImage.fill(false);
}

bool Material::save(bool recursive)
{
    if (recursive)
        for (int i = 0; i < m_useImage.size(); i++) {
            if (m_useImage[i]) {
                m_images[i].getResource().save();
            }
        }
    return ResourceSerializer::serialize(*this);
}

bool Material::load(YAML::Node& data)
{
    if (!data["Type"] || data["Type"].as<std::string>() != std::string("Material")) {
        GLX_CORE_ERROR("File unsupported");
        return false;
    }

    std::unordered_map<TextureType, std::string> textureTypeToStr;
    textureTypeToStr[ALBEDO]    = "Albedo";
    textureTypeToStr[ROUGHNESS] = "Roughness";
    textureTypeToStr[METALLIC]  = "Metallic";
    textureTypeToStr[NORMAL]    = "Normal";
    textureTypeToStr[AO]        = "Ao";
    auto images                 = data["Images"];
    for (auto& keyVal : textureTypeToStr) {
        if (images[keyVal.second]) {
            m_useImage[keyVal.first] = true;
            uuid imageID             = images[keyVal.second].as<uint64_t>();
            m_images[keyVal.first]   = ResourceManager::getInstance().load<Image>(Project::getPath(ProjectPathTypes::RESOURCE, imageID));
        }
    }

    if (data["Constants"]) {
        m_albedo = data["Constants"]["Albedo"].as<vec3>();
    }

    return true;
}

void Material::setImage(TextureType type, ResourceHandle<Image> image)
{
    m_useImage[type] = true;
    m_images[type]   = image;
}

} // namespace Galaxy
