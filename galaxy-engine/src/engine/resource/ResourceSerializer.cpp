#include "ResourceSerializer.hpp"

#include "Image.hpp"
#include "Log.hpp"
#include "Material.hpp"
#include "Mesh.hpp"
#include "common/YamlTranslation.hpp"
#include "project/Project.hpp"

#include <fstream>
#include <yaml-cpp/yaml.h>

namespace Galaxy {
bool ResourceSerializer::serialize(Image& image)
{
    GLX_CORE_INFO("Serializing: {0}", image.getExternalFilePath());
    YAML::Emitter yaml;
    yaml << YAML::BeginMap;
    yaml << YAML::Key << "Type" << YAML::Value << "Image";

    if (!image.isInternal()) {
        yaml << YAML::Key << "ExternalFile" << YAML::Value << image.getExternalFilePath();
    }

    yaml << YAML::EndMap;

    std::ofstream fout(Project::getProjectRootPath() + Project::getPath(ProjectPathTypes::RESOURCE, image.getResourceID()));
    fout << yaml.c_str();
    return true;
}
bool ResourceSerializer::serialize(Material& material)
{
    GLX_CORE_INFO("Serializing: {0}", material.getPath());

    YAML::Emitter yaml;
    yaml << YAML::BeginMap;
    yaml << YAML::Key << "Type" << YAML::Value << "Material";

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
    yaml << YAML::Key << "Albedo" << YAML::Value << material.m_albedo;
    yaml << YAML::EndMap;

    std::ofstream fout(Project::getProjectRootPath() + Project::getPath(ProjectPathTypes::RESOURCE, material.getResourceID()));
    fout << yaml.c_str();
    return true;
}
bool ResourceSerializer::serialize(Mesh& mesh)
{
    GLX_CORE_INFO("Serializing: {0}", mesh.getPath());

    YAML::Emitter yaml;
    yaml << YAML::BeginMap;
    yaml << YAML::Key << "Type" << YAML::Value << "Mesh";

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
    yaml << YAML::EndMap;

    std::ofstream fout(Project::getProjectRootPath() + Project::getPath(ProjectPathTypes::RESOURCE, mesh.getResourceID()));
    fout << yaml.c_str();
    return true;
}
} // namespace Galaxy
