#include "ResourceDeserializer.hpp"

#include "Image.hpp"
#include "Log.hpp"
#include "Mesh.hpp"
#include "project/Project.hpp"

namespace Galaxy {
bool ResourceDeserializer::deserialize(Image& image, YAML::Node& data)
{
    if (!data["Type"] || data["Type"].as<std::string>() != std::string("Image")) {
        GLX_CORE_ERROR("File unsupported");
        return false;
    }

    if (data["ExternalFile"]) {
        return image.loadExtern(data["ExternalFile"].as<std::string>());
    }

    return false;
}
bool ResourceDeserializer::deserialize(Mesh& mesh, YAML::Node& data)
{
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
