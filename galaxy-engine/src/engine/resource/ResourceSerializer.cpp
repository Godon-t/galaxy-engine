#include "ResourceSerializer.hpp"

#include "Image.hpp"

#include <fstream>
#include <yaml-cpp/yaml.h>

namespace Galaxy {
bool ResourceSerializer::serialize(Image& image, std::string& outputPath)
{
    YAML::Emitter yaml;
    yaml << YAML::BeginMap;
    yaml << YAML::Key << "Type" << YAML::Value << "Image";

    if (!image.isInternal()) {
        yaml << YAML::Key << "ExternalFile" << YAML::Value << image.getExternalFilePath();
    }

    yaml << YAML::EndMap;

    std::ofstream fout(outputPath);
    fout << yaml.c_str();
    return false;
}
} // namespace Galaxy
