#include "ResourceDeserializer.hpp"

#include "Image.hpp"
#include "Log.hpp"
#include "Mesh.hpp"
#include "project/Project.hpp"

#include <fstream>
#include <yaml-cpp/yaml.h>

namespace Galaxy {
bool ResourceDeserializer::deserialize(Image& image, const std::string& path)
{
    YAML::Node data;

    std::ifstream stream(Project::getProjectRootPath() + path);
    std::stringstream strStream;
    strStream << stream.rdbuf();

    data = YAML::Load(strStream.str());

    if (!data["Type"] || data["Type"].as<std::string>() != std::string("Image")) {
        GLX_CORE_ERROR("File unsupported");
        return false;
    }

    if (data["ExternalFile"]) {
        return image.loadExtern(data["ExternalFile"].as<std::string>());
    }

    return false;
}
bool ResourceDeserializer::deserialize(Mesh& mesh, const std::string& filePath)
{
    YAML::Node data;

    std::ifstream stream(Project::getProjectRootPath() + filePath);
    std::stringstream strStream;
    strStream << stream.rdbuf();

    data = YAML::Load(strStream.str());

    if (!data["Type"] || data["Type"].as<std::string>() != std::string("Mesh")) {
        GLX_CORE_ERROR("File unsupported");
        return false;
    }

    if (data["ExternalFile"]) {
        return mesh.loadExtern(data["ExternalFile"].as<std::string>());
    }

    return false;
}
} // namespace Galaxy
