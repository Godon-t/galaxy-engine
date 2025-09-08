#pragma once

#include <yaml-cpp/yaml.h>

namespace Galaxy {
class Image;
class Mesh;

class ResourceDeserializer {
public:
    static bool deserialize(Image& image, YAML::Node& data);
    static bool deserialize(Mesh& image, YAML::Node& data);
};
} // namespace Galaxy
