#include "ResourceDeserializer.hpp"

#include "Image.hpp"
#include "Log.hpp"

#include <fstream>
#include <yaml-cpp/yaml.h>

namespace Galaxy {
bool ResourceDeserializer::deserialize(Image& image, const std::string& path)
{
    YAML::Node data;

    std::ifstream stream(path);
    std::stringstream strStream;
    strStream << stream.rdbuf();

    data = YAML::Load(strStream.str());

    if (!data["Type"] || data["Type"].as<std::string>() != std::string("Image")) {
        GLX_CORE_ERROR("File unsupported");
        return false;
    }

    if (data["ExternalFile"]) {
        image.m_isInternal = false;
        return image.loadExtern(data["ExternalFile"].as<std::string>());
    }

    return false;
}
} // namespace Galaxy
