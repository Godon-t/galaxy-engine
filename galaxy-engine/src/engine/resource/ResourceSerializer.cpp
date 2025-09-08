#include "ResourceSerializer.hpp"

#include "Image.hpp"
#include "Mesh.hpp"
#include "project/Project.hpp"

#include <fstream>
#include <yaml-cpp/yaml.h>

namespace Galaxy {
bool ResourceSerializer::serialize(Image& image)
{
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
bool ResourceSerializer::serialize(Mesh& mesh)
{
    YAML::Emitter yaml;
    yaml << YAML::BeginMap;
    yaml << YAML::Key << "Type" << YAML::Value << "Mesh";

    if (!mesh.isInternal()) {
        yaml << YAML::Key << "ExternalFile" << YAML::Value << mesh.getExternalFilePath();
    }

    yaml << YAML::EndMap;

    std::ofstream fout(Project::getProjectRootPath() + Project::getPath(ProjectPathTypes::RESOURCE, mesh.getResourceID()));
    fout << yaml.c_str();
    return true;
}
} // namespace Galaxy
