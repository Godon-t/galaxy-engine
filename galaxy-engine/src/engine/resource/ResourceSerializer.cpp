#include "ResourceSerializer.hpp"

#include "Environment.hpp"
#include "Image.hpp"
#include "Log.hpp"
#include "Material.hpp"
#include "Mesh.hpp"
#include "common/YamlTranslation.hpp"
#include "project/Project.hpp"

#include <utility>
#include <algorithm>
#include <fstream>
#include <yaml-cpp/yaml.h>

namespace Galaxy {

void openResourceWriting(YAML::Emitter& yaml, const std::string& path, const char* type)
{
    GLX_CORE_INFO("Serializing: {0}", path);
    yaml << YAML::BeginMap;
    yaml << YAML::Key << "Type" << YAML::Value << type;
}

bool closeResourceWriting(YAML::Emitter& emitter, uuid resourceID)
{
    emitter << YAML::EndMap;

    std::ofstream fout(Project::getProjectRootPath() + Project::getPath(ProjectPathTypes::RESOURCE, resourceID));
    fout << emitter.c_str();
    return fout.good();
}

bool ResourceSerializer::serialize(Image& image)
{
    YAML::Emitter yaml;
    openResourceWriting(yaml, image.getPath(), "Image");

    if (!image.isInternal()) {
        yaml << YAML::Key << "ExternalFile" << YAML::Value << image.getExternalFilePath();
    }

    return closeResourceWriting(yaml, image.getResourceID());
}

bool ResourceSerializer::serialize(Environment& env)
{
    YAML::Emitter yaml;
    openResourceWriting(yaml, env.getPath(), "Environment");

    yaml << YAML::Key << "Skybox" << YAML::BeginSeq;
    for (auto& image : env.m_skybox) {
        yaml << image.getResource().getResourceID();
    }
    yaml << YAML::EndSeq;

    return closeResourceWriting(yaml, env.getResourceID());
}

bool ResourceSerializer::serialize(BaseMaterial& material)
{
    switch (material.getType()) {
    case MaterialType::PBR:
        return serialize(static_cast<PBRMaterial&>(material));
    case MaterialType::Texture:
        return serialize(static_cast<TextureMaterial&>(material));
    }
    return false;
}


bool ResourceSerializer::serialize(PBRMaterial& material)
{
    YAML::Emitter yaml;
    openResourceWriting(yaml, material.getPath(), "PBRMaterial");

    yaml << YAML::Key << "HasTransparency" << YAML::Value << material.isUsingTransparency();
    yaml << YAML::Key << "Transparency" << YAML::Value << material.getTransparency();

    yaml << YAML::Key << "Images" << YAML::BeginMap;
    std::unordered_map<TextureType, std::string> textureTypeToStr;
    textureTypeToStr[ALBEDO]    = "Albedo";
    textureTypeToStr[ROUGHNESS] = "Roughness";
    textureTypeToStr[METALLIC]  = "Metallic";
    textureTypeToStr[NORMAL]    = "Normal";
    textureTypeToStr[AO]        = "Ao";
    for (auto& keyVal : textureTypeToStr) {
        if (material.canUseImage(keyVal.first)) {
            yaml << YAML::Key << keyVal.second << YAML::Value << material.getImage(keyVal.first).getResource().getResourceID();
        }
    }
    yaml << YAML::EndMap;

    yaml << YAML::Key << "Constants" << YAML::BeginMap;
    yaml << YAML::Key << "Albedo" << YAML::Value << material.getAlbedo();
    yaml << YAML::Key << "Ambient" << YAML::Value << material.getAmbient();
    yaml << YAML::Key << "Roughness" << YAML::Value << material.getRoughness();
    yaml << YAML::Key << "Metallic" << YAML::Value << material.getMetallic();
    yaml << YAML::EndMap;

    return closeResourceWriting(yaml, material.getResourceID());
}

bool ResourceSerializer::serialize(TextureMaterial& material)
{
    const auto* image = std::get_if<ResourceHandle<Image>>(&material.getTexture());
    if (material.hasTexture() && !image) {
        GLX_CORE_ERROR("Cannot serialize a texture material backed by a GPU texture or cubemap");
        return false;
    }

    YAML::Emitter yaml;
    openResourceWriting(yaml, material.getPath(), "TextureMaterial");

    yaml << YAML::Key << "HasTransparency" << YAML::Value << material.isUsingTransparency();
    yaml << YAML::Key << "Transparency" << YAML::Value << material.getTransparency();
    yaml << YAML::Key << "HasTexture" << YAML::Value << material.hasTexture();
    if (material.hasTexture())
        yaml << YAML::Key << "ImagePath" << YAML::Value << image->getResource().getPath();

    return closeResourceWriting(yaml, material.getResourceID());
}

bool ResourceSerializer::serialize(Mesh& mesh)
{
    YAML::Emitter yaml;
    openResourceWriting(yaml, mesh.getPath(), "Mesh");

    if (!mesh.isInternal()) {
        yaml << YAML::Key << "ExternalFile" << YAML::Value << mesh.getExternalFilePath();
    }

    yaml << YAML::Key << "SubMeshes" << YAML::BeginSeq;
    for (int i = 0; i < mesh.getSubMeshesCount(); i++) {
        yaml << YAML::BeginMap;
        yaml << YAML::Key << "HasMaterial" << YAML::Value << mesh.hasMaterial(i);
        if (mesh.hasMaterial(i)) {
            yaml << YAML::Key << "MaterialID" << YAML::Value << mesh.getMaterial(i).getResource().m_resourceID;
        }
        yaml << YAML::EndMap;
    }
    yaml << YAML::EndSeq;

    return closeResourceWriting(yaml, mesh.getResourceID());
}
} // namespace Galaxy
