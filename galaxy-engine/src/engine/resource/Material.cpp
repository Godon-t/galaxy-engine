#include "Material.hpp"

#include "ResourceManager.hpp"
#include "common/YamlTranslation.hpp"
#include "core/Log.hpp"

namespace Galaxy {

BaseMaterial::BaseMaterial(BaseMaterial&& other) noexcept
    : m_useTransparency(other.m_useTransparency)
    , m_transparency(other.m_transparency)
    , m_materialHandle(std::exchange(other.m_materialHandle, {}))
{
}

bool BaseMaterial::save(bool recursive)
{
    return ResourceSerializer::serialize(*this);
}

bool BaseMaterial::load(YAML::Node& data)
{
    float transparency = 1.0f;
    const auto constants = data["Constants"];
    if (constants && constants["Transparency"])
        transparency = constants["Transparency"].as<float>();

    setTransparency(data["Transparency"].as<float>(transparency));
    setUseTransparency(data["HasTransparency"].as<bool>(getTransparency() < 1.0f));
    return true;
}

void PBRMaterial::setImage(TextureType type, ResourceHandle<Image> image)
{
    m_useImage[type] = true;
    m_images[type] = image;
}

bool PBRMaterial::load(YAML::Node& data)
{
    const auto type = data["Type"].as<std::string>("");
    if (type != "PBRMaterial") {
        GLX_CORE_ERROR("File unsupported");
        return false;
    }

    BaseMaterial::load(data);

    std::unordered_map<TextureType, std::string> textureTypeToStr;
    textureTypeToStr[ALBEDO]    = "Albedo";
    textureTypeToStr[ROUGHNESS] = "Roughness";
    textureTypeToStr[METALLIC]  = "Metallic";
    textureTypeToStr[NORMAL]    = "Normal";
    textureTypeToStr[AO]        = "Ao";
    m_useImage.fill(false);
    m_images = {};
    const auto images = data["Images"];
    if (images) {
        for (const auto& keyVal : textureTypeToStr) {
            if (images[keyVal.second]) {
                uuid imageID = images[keyVal.second].as<uint64_t>();
                setImage(keyVal.first, ResourceManager::getInstance().load<Image>(Project::getPath(ProjectPathTypes::RESOURCE, imageID)));
            }
        }
    }

    const auto constants = data["Constants"];
    if (constants) {
        setAlbedo(constants["Albedo"].as<vec3>(getAlbedo()));
        setAmbient(constants["Ambient"].as<float>(getAmbient()));
        setRoughness(constants["Roughness"].as<float>(getRoughness()));
        setMetallic(constants["Metallic"].as<float>(getMetallic()));
    }

    return true;
}

bool TextureMaterial::load(YAML::Node& data)
{
    if (data["Type"].as<std::string>("") != "TextureMaterial") {
        GLX_CORE_ERROR("File unsupported");
        return false;
    }

    BaseMaterial::load(data);
    m_texture = ResourceHandle<Image> {};
    m_hasTexture = false;
    const auto imagePath = data["ImagePath"];
    if (data["HasTexture"].as<bool>(imagePath.IsDefined())) {
        if (!imagePath) {
            GLX_CORE_ERROR("Texture material has no image resource path");
            return false;
        }
        setTexture(ResourceManager::getInstance().load<Image>(imagePath.as<std::string>()));
    }
    return true;
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
